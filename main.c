#include "main.h"
char s[MAX_STRING_SIZE];
void doInit(){
	inputFile = bfd_openr(INPUT_FILE_NAME,NULL);
	if (inputFile==0){
		sprintf(s,"Error opening %s\n",INPUT_FILE_NAME);
		bfd_perror(s);
		exit(1);
	}
	if (!bfd_check_format (inputFile, bfd_object)) {
		printf("Failed to open object file!\n");
		exit(-1);        
    }
}
void getSymbols(bfd*abfd){
	size_t x = bfd_get_symtab_upper_bound(abfd);
	symbols = malloc(x);
	symCount = bfd_canonicalize_symtab(abfd, symbols);
}
void findFunctions(){
	for (int x = 0; x<symCount; x++) {
		if (strcmp(symbols[x]->name, "puts")==0) {
			// printf("Changing address for undefined symbol: %s\n", symbols[x]->name);
			symbols[x]->value = (long unsigned int) ADDRESS_OF_PUTS;
		}
		// printf("%s\n",symbols[x]->name);
		if (strcmp(symbols[x]->name, "app_main")==0) {
			// printf("Storing address of symbol: %s!\n", symbols[x]->name);
			app_main = textStart + symbols[x]->value;
		}
	}
}
void relocateSection(bfd*abfd, asection*section){
	char **error_message=malloc(0x1000);
	asection *literalAppMain;
	asymbol *symbol;
	if (strstr(section->name,"debug")){
		return;
	}
	bfd_reloc_status_type type;
	long relSize = bfd_get_reloc_upper_bound (abfd, section);
	if (relSize < 0) {
		bfd_perror("bfd_get_reloc_upper_bound");
		exit(EXIT_FAILURE);
	}else if (relSize == 0) {
		return;  
	}
	arelent **relocs  = malloc (relSize);
	if (!relocs) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	long relcount = bfd_canonicalize_reloc (abfd, section, relocs , symbols);
	if (relcount < 0) {
		// return;
		bfd_perror("bfd_canonicalize_reloc");
		exit(EXIT_FAILURE);
	};
	printf("Processing relocations for section: %s\n", section->name);
	for (int x = 0; x<relcount; x++) {
		arelent *reloc = relocs[x];
		if(reloc->howto==0){
			bfd_perror("unable to set reloc type");
			exit(1);
		}
		reloc_howto_type *howto = reloc->howto;
		literalAppMain = bfd_get_section_by_name(abfd,".literal.app_main");
		symbol = *reloc->sym_ptr_ptr;
		symbol->value=literalAppMain->vma;symbol->section;
		printf("Relocation %d: offset=0x%lx, symbol=%s, type=%d %s\n",
			x, (unsigned long)reloc->address, symbol->name, howto->type, reloc->howto->name);
		// section->output_section = section;section->output_offset;
		section->output_section = (*reloc->sym_ptr_ptr)->section;//section->lm
		// type = bfd_perform_relocation(abfd, reloc, (void*)section->vma, section, NULL, error_message);
		type = bfd_perform_relocation(abfd, reloc, (void*)section->contents, section, NULL, error_message);
	}
	free (relocs);
}

void relocateAll(bfd*abfd){
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (!(section->flags & SEC_RELOC)) {
			// Skip sections that don't have relocations
			continue;
        }
		if(strstr(section->name,".xt.")){
			continue;
		}
		printf("%s\n",section->name);
		// section->lma = section->vma = (bfd_vma)section->contents;
		// if (strcmp(section->name,".text.app_main")==0){ //remove condition
		relocateSection(inputFile,section);
		// }
	}
}

size_t roundUpToMultipleOf(size_t value, size_t size) {
  size_t r = value % size;
  return r == 0 ? value : value - r + size;
}

int getMemorySizeAndSetVmaOffsets(bfd *abfd){
	int rodataSize=0,literalSize=0,textSize=0;
	rodataStart = 0;
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,RODATA_SUBSTRING)){
			section->vma=rodataStart+rodataSize;
			rodataSize+=section->size;
			printf("Add Memory for %s\n",section->name);
		} 
	}
	rodataSize=roundUpToMultipleOf(rodataSize,ALLIGN);
	literalStart=rodataSize;
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,LITERAL_SUBSTRING)){
			section->vma=literalStart+literalSize;
			literalSize+=section->size;
			printf("Add Memory for %s\n",section->name);
		}
	}
	literalSize=roundUpToMultipleOf(literalSize,ALLIGN);
	textStart+=literalStart+literalSize;
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,TEXT_SUBSTRING)){
			section->vma=textStart+textSize;
			textSize+=section->size;
			printf("Add Memory for %s\n",section->name);
		}
	}
	textSize=roundUpToMultipleOf(textSize,ALLIGN);
	return rodataSize+literalSize+textSize;
}
void setSymbolValue(const char*symbolName,bfd_vma vma){
	for(int x=0;x<symCount;x++){
		if(strcmp(symbols[x]->name,symbolName)==0){
			symbols[x]->value=vma;
		}
	}
}

void putSubSectionsInMemoryAndRelocateTargetVma(bfd *abfd,char* subString,uintptr_t hostStart,uintptr_t targetStart){
	uintptr_t hostAddress = hostStart;
	bfd_boolean test;
	for(asection* section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,subString)){
			test = bfd_get_section_contents(abfd,section,(void*)hostAddress,0,section->size);
			section->contents = (unsigned char *) hostAddress;
			section->vma+=targetStart;
			setSymbolValue(section->name,section->vma);
			hostAddress+=section->size;
		}
	}
}
void putAllInMemory(bfd*abfd){
	int memorySize=	getMemorySizeAndSetVmaOffsets(abfd);
	memoryStart =(uintptr_t) malloc(memorySize);
	// uintptr_t memoryStartAligned =roundUpToMultipleOf(memoryStart,ALLIGN);
	// uintptr_t hostRodataStart 	= rodataStart + memoryStartAligned;
	// uintptr_t hostLiteralStart	= literalStart + memoryStartAligned;
	// uintptr_t hostTextStart		= textStart + memoryStartAligned;
	putSubSectionsInMemoryAndRelocateTargetVma(abfd,RODATA_SUBSTRING,memoryStart+rodataStart,ADDRESS_OF_RODATA);
	putSubSectionsInMemoryAndRelocateTargetVma(abfd,LITERAL_SUBSTRING,memoryStart+literalStart,ADDRESS_OF_LITERAL);
	putSubSectionsInMemoryAndRelocateTargetVma(abfd,TEXT_SUBSTRING,memoryStart+textStart,ADDRESS_OF_TEXT);
}
void process(bfd*abfd){
	getSymbols(abfd);
	findFunctions();
	putAllInMemory(abfd);
	relocateAll(abfd);
}
void outputSectionBin(asection* section){
	FILE *f = fopen(OUTPUT_FILE_NAME,"w");
	fwrite(section->contents,section->size,1,f);
	for(int x=0;x<section->size;x++){
		printf("0x%02x ",*(section->contents+x));
	}
	printf("\n");
}
void outputBin(){
	asection* section;
	FILE *f = fopen(OUTPUT_FILE_NAME,"w");
	section = bfd_get_section_by_name(inputFile,".rodata.app_main.str1.4");
	fwrite(section->contents,section->size,1,f);
	section = bfd_get_section_by_name(inputFile,".literal.app_main");
	fwrite(section->contents,section->size,1,f);
	section = bfd_get_section_by_name(inputFile,".text.app_main");
	fwrite(section->contents,section->size,1,f);
}
void main() {
	// if (true){
	if (false){
		main2();
		exit(1);
	}
	doInit();
	process(inputFile);
	outputBin();
	// asection* section;
	// section = bfd_get_section_by_name(inputFile,".rodata.app_main.str1.4");
	// outputSectionBin(section);
	// section = bfd_get_section_by_name(inputFile,".literal.app_main");
	// outputSectionBin(section);
	// section = bfd_get_section_by_name(inputFile,".text.app_main");
	// outputSectionBin(section);
	printf("Fin\n");
    return;
}
