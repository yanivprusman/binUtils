#include "main.h"
char s[MAX_STRING_SIZE];
void doInit(){
	bfd_boolean ok;
	inputFile = bfd_openr(INPUT_FILE_NAME,NULL);
	if (inputFile==0){
		sprintf(s,"Error opening %s\n",INPUT_FILE_NAME);
		bfd_perror(s);
		exit(1);
	}
	ok = bfd_check_format (inputFile, bfd_object);
	if (!ok) {
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
			symbols[x]->value = (long unsigned int) addressOfPuts;
		}
		// printf("%s\n",symbols[x]->name);
		if (strcmp(symbols[x]->name, "app_main")==0) {
			// printf("Storing address of symbol: %s!\n", symbols[x]->name);
			app_main = textEspStart + symbols[x]->value;
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
		symbol = *reloc->sym_ptr_ptr;
		printf("Relocation %d: offset=0x%lx, symbol=%s, type=%d %s\n",
			x, (unsigned long)reloc->address, symbol->name, howto->type, reloc->howto->name);
		if (strstr(section->name,TEXT_SUBSTRING)){
			section->output_section = section; 
		}else{
			section->output_section = symbol->section;
		}
		type = bfd_perform_relocation(abfd, reloc, (void*)section->contents, section, NULL, error_message);
		if (type!=bfd_reloc_ok){
			printf("**************************************************************************************\n");
			printf("Relocation error: %s\n", error_message[0]);
			exit(1);
		}
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

int getMemorySize(bfd *abfd){
	int size = 0;
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,RODATA_SUBSTRING)){
			size+=section->size;
			size=roundUpToMultipleOf(size,ALIGN);
		} 
	}
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,LITERAL_SUBSTRING)){
			size+=section->size;
			size=roundUpToMultipleOf(size,ALIGN);
		} 
	}
	for(asection*section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,TEXT_SUBSTRING)){
			size+=section->size;
			size=roundUpToMultipleOf(size,ALIGN);
		} 
	}
	return size;
}
void setSymbolValue(const char*symbolName,bfd_vma vma){
	for(int x=0;x<symCount;x++){
		if(strcmp(symbols[x]->name,symbolName)==0){
			symbols[x]->value=vma;
		}
	}
}

void putSubSectionsInMemoryAndSetThereVma(bfd *abfd,char* subString){
	bfd_boolean test;
	bfd_boolean ok;
	for(asection* section = abfd->sections; section!=NULL; section = section->next){
		if (strstr(section->name,subString)){
			nextMemoryAddress = roundUpToMultipleOf(nextMemoryAddress,ALIGN);
			nextMemoryAddressEsp = roundUpToMultipleOf(nextMemoryAddressEsp,ALIGN);
			test = bfd_get_section_contents(abfd,section,(void*)nextMemoryAddress,0,section->size);
			section->contents = (unsigned char *) nextMemoryAddress;
			section->lma= section->vma=nextMemoryAddressEsp;
			ok = bfd_set_section_vma(abfd,section,nextMemoryAddressEsp);
			// bfd_get_section_lma()
			if(!ok) {
				printf("**************************************************************************************\n");
				bfd_perror("error Setting vma");

			}
			nextMemoryAddress+=section->size;
			nextMemoryAddressEsp+=section->size;
			setSymbolValue(section->name,section->vma);
		}
	}
}
void putAllInMemory(bfd*abfd){
	int memorySize=	getMemorySize(abfd);
	memoryStart = (uintptr_t) malloc(memorySize+ALIGN);
	memoryStart = roundUpToMultipleOf(memoryStart,ALIGN);
	memoryStartEsp = 0x4038baa0;//get addresses of memory from esp
	addressOfPuts  = 0x4200d6cc;//get addresses of functions(puts) from esp
	nextMemoryAddress = memoryStart;
	nextMemoryAddressEsp = memoryStartEsp;
	rodataHostStart=roundUpToMultipleOf(nextMemoryAddress,ALIGN);
	rodataEspStart=roundUpToMultipleOf(nextMemoryAddressEsp,ALIGN);
	putSubSectionsInMemoryAndSetThereVma(abfd,RODATA_SUBSTRING);
	literalHostStart=roundUpToMultipleOf(nextMemoryAddress,ALIGN);
	literalEspStart=roundUpToMultipleOf(nextMemoryAddressEsp,ALIGN);
	putSubSectionsInMemoryAndSetThereVma(abfd,LITERAL_SUBSTRING);
	textHostStart=roundUpToMultipleOf(nextMemoryAddress,ALIGN);
	textEspStart=roundUpToMultipleOf(nextMemoryAddressEsp,ALIGN);
	putSubSectionsInMemoryAndSetThereVma(abfd,TEXT_SUBSTRING);
}
void process(bfd*abfd){
	getSymbols(abfd);
	findFunctions();
	putAllInMemory(abfd);
	findFunctions();
	relocateAll(abfd);
}
// void outputSectionBin(asection* section){
// 	FILE *f = fopen(OUTPUT_FILE_NAME,"w");
// 	fwrite(section->contents,section->size,1,f);
// 	for(int x=0;x<section->size;x++){
// 		printf("0x%02x ",*(section->contents+x));
// 	}
// 	printf("\n");
// }
void outputBin(){
	asection* section;
	FILE *f = fopen(OUTPUT_FILE_NAME,"w");
	fwrite((void*)memoryStart,nextMemoryAddress-memoryStart,1,f);
	printf("//Rodata:\n//");
	for(uintptr_t x=0;x<literalHostStart-rodataHostStart;x++){
		printf("%c",(unsigned char)*((char*)rodataHostStart + x));
	};
	printf("\n");
	for(uintptr_t x=0;x<literalHostStart-rodataHostStart;x++){
		printf("0x%02x, ",(unsigned char)*((char*)rodataHostStart + x));
	};
	printf("\n");
	printf("//Literal:");
	for(uintptr_t x=0;x<textHostStart-literalHostStart;x++){
		if (!(x%4)) printf("\n");
		printf("0x%02x, ",(unsigned char)*((char*)literalHostStart + x));
	};
	printf("\n");
	printf("//Text:");
	for(uintptr_t x=0;x<nextMemoryAddress-textHostStart;x++){
		if (!(x%3)) printf("\n");
		printf("0x%02x, ",(unsigned char)*((char*)textHostStart + x));
	}
	printf("\n");
}
void printSymbols(){
	printf("Symbols:\n");
	for (int x = 0; x<symCount; x++) {
		printf("%s %lx\n",symbols[x]->name,symbols[x]->value);
	}
}
void handle_pc_relative_relocation(bfd *abfd, asection *section, arelent *reloc_entry, bfd_vma pc) {
    bfd_vma symbol_value;
    bfd_vma relocation_offset;
    bfd_vma pc_relative_offset;
    symbol_value = bfd_asymbol_value(reloc_entry->sym_ptr_ptr[0]);
    relocation_offset = reloc_entry->address;
    pc_relative_offset = symbol_value - (pc + relocation_offset);
    bfd_put_32(abfd, pc_relative_offset, section->contents + relocation_offset);
    printf("Relocation: Symbol = 0x%lx, PC = 0x%lx, Relocation Offset = 0x%lx\n",
           (unsigned long)symbol_value, (unsigned long)pc, (unsigned long)relocation_offset);
    printf("Computed PC-relative offset: 0x%lx\n", (unsigned long)pc_relative_offset);
}
void main() {
	if (false){
		main2();
		exit(1);
	}
	doInit();
	process(inputFile);
	outputBin();
	printSymbols();
	printf("Fin\n");
    return;
}
