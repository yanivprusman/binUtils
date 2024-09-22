#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <bfd.h>
// #include "/mnt/c/101_coding/118_binutils-gdb/include/elf/xtensa.h"
// #include "elf/xtensa.h" "C:\101_coding\118_binutils-gdb\bfd\elf32-xtensa.c"
// #include "/mnt/c/101_coding/118_binutils-gdb/bfd/elf32-xtensa.c"
// #include <include/xtensa-config.h>
// #define INPUT_FILE_NAME "/mnt/c//101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define INPUT_FILE_NAME "test_unit.o"
void *data_start;
void *literal_start;
void *text_start;
bfd *abfd;
asection *data,*literal,*text;
asymbol **symbols;
int myCallback(int a) {
	printf("myCallback called!\n");
	return a*4;
}
typedef int (*t_callback)(int);
typedef void (*t_test_function)(int, int*);
t_callback my_callback = myCallback;
const char *test_function_name = "app_main";
int in = 10;
int out[2] = {0,0};
t_test_function test_function;
typedef void *ptr_t;
typedef unsigned char byte_t;
static byte_t *memory = NULL;
static size_t size = 0;
static size_t pagesize;
size_t _round_up_to_multiple_of(size_t value, size_t size) {
  size_t r = value % size;
  return r == 0 ? value : value - r + size;
}
void alloc_rwx(size_t minMemSize) {
  size_t pagesize = sysconf(_SC_PAGESIZE);
  if (minMemSize <= pagesize)
    size = pagesize;
  else
    size = _round_up_to_multiple_of(size, pagesize);
    
  if (posix_memalign((void**)&memory, pagesize, size) != 0)
    exit(1); // TODO: error msg
  if (mprotect(memory, size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    exit(1); // TODO: error msg
  memset(memory, 0, size);
}

void relocate(asection*section){
	void *text_start2=text_start;
	long relsize = bfd_get_reloc_upper_bound (abfd, section);
	arelent **relpp = malloc (relsize);
	long relcount = bfd_canonicalize_reloc (abfd, section, relpp, symbols);
	char *tmp;
	bfd_reloc_status_type type;
	// literal->vma=0x11111111;
	// data->vma=0x22222222;
	for (int x = 0; x<relcount; x++) {
		arelent *reloc = relpp[x];
		if (!strcmp((*reloc->sym_ptr_ptr)->name, "callback")) {
			printf("Found relocation for symbol: %s!\n", (*reloc->sym_ptr_ptr)->name);
			section->output_section = (*reloc->sym_ptr_ptr)->section;
			bfd_perform_relocation(abfd, reloc, (void*)section->vma, section, NULL, &tmp);
		}
		if (!strcmp((*reloc->sym_ptr_ptr)->name, ".rodata.app_main.str1.4")) {
			printf("found relocation .rodata.app_main.str1.4\n");
			// (*reloc->sym_ptr_ptr)->value=(uint32_t)(uintptr_t)data_start;
		}
		if (!strcmp((*reloc->sym_ptr_ptr)->name, ".literal.app_main")) {
			printf("found relocation .literal.app_main\n");
			// (*reloc->sym_ptr_ptr)->value=(uint32_t)(uintptr_t)literal_start;
		}
		if (!strcmp((*reloc->sym_ptr_ptr)->name, "puts")) {
			printf("found relocation puts\n");
			// (*reloc->sym_ptr_ptr)->value=(uint32_t)(uintptr_t)my_callback;

		}
		section->output_section = section;
		// section->output_section = (*reloc->sym_ptr_ptr)->section;
		// BFD_RELOC_X86_64_PLT32
		const bfd_arch_info_type *arch_info = bfd_get_arch_info(abfd);
		reloc->howto = bfd_reloc_type_lookup(abfd, BFD_RELOC_XTENSA_GLOB_DAT);
		if(reloc->howto==0){
			bfd_perror("unable to set reloc type");
		}
		type = bfd_perform_relocation(abfd, reloc, (void*)section->vma, section, abfd, &tmp);
		type = bfd_reloc_dangerous;
	}
	free (relpp);
}
void printSupportedFormats(){
    const char **target_list = bfd_target_list();
    if (target_list == NULL) {
        printf("Error: No supported targets found.\n");
        return;
    }

    int found_xtensa = 0;

    printf("Supported BFD formats:\n");

    // Loop through the target list and print each target name
    for (int i = 0; target_list[i] != NULL; i++) {
        printf(" - %s\n", target_list[i]);
        if (strstr(target_list[i], "xtensa") != NULL) {
            found_xtensa = 1;
        }
    }

    if (found_xtensa) {
        printf("Xtensa is supported.\n");
    } else {
        printf("Xtensa is not supported.\n");
    }
}

int main(int argc, char **argv) {
	enum bfd_architecture;
	bfd_init();
	// xtensa_elf32_be_vec a;
	
	// printSupportedFormats();
    printf("loading %s.\n",INPUT_FILE_NAME);
	abfd = bfd_openr(INPUT_FILE_NAME, NULL);
	// abfd = bfd_openr(INPUT_FILE_NAME, "elf32-xtensa");
	// abfd = bfd_openr(INPUT_FILE_NAME, "elf32-xtensa-le");

	bfd_arch_info_type *archInfo;
	abfd->arch_info->printable_name;
	// a = bfd_get_arch_info(abfd);
	archInfo=bfd_scan_arch("xtensa");
	bfd_cleanup cleanup;
	bfd_arch_xtensa ;
	
	cleanup=abfd->xvec->_bfd_check_format;
	bfd_set_arch_info(abfd, bfd_scan_arch("architecture-name"));
	const char *target_name = bfd_get_target(abfd);
	bfd_format format;
	bool ok;
	ok = bfd_set_default_target("xtensa");
	printf("Target name: %s\n", target_name);
	if (abfd==0){
		bfd_perror("cant open elf32-xtensa");
	}
	if (!bfd_check_format (abfd, bfd_object)) {
		printf("Failed to open object file!\n");
		exit(-1);        
    }
	literal = bfd_get_section_by_name (abfd, ".literal.app_main");
	text = bfd_get_section_by_name (abfd, ".text.app_main");
	data = bfd_get_section_by_name (abfd, ".rodata.app_main.str1.4");
	size_t minMemSize = data->size + literal->size + text->size;
	alloc_rwx(minMemSize);
	bfd_get_section_contents(abfd, data, memory, 0, data->size);
	bfd_get_section_contents(abfd, literal, memory + data->size, 0, literal->size);
	bfd_get_section_contents(abfd, text, memory + data->size + literal->size, 0, text->size);
	data_start = memory;
	literal_start = memory + data->size;
	text_start = memory + literal->size + data->size;
	
	data->vma = data->output_offset = (long unsigned int) data_start;
	literal->vma = literal->output_offset = (long unsigned int) literal_start;
	text->vma = text->output_offset = (long unsigned int) text_start;
	size_t symsize = bfd_get_symtab_upper_bound(abfd);
	symbols = malloc(symsize);
	int symcount = bfd_canonicalize_symtab(abfd, symbols);
	for (int x = 0; x<symcount; x++) {
		if (!strcmp(symbols[x]->name, "puts")) {
			printf("Changing address for undefined symbol: %s\n", symbols[x]->name);
			symbols[x]->value = (long unsigned int) my_callback;
		}
		if (!strcmp(symbols[x]->name, ".rodata.app_main.str1.4")) {
			printf("Changing address for symbol: %s\n", symbols[x]->name);
			symbols[x]->value = (long unsigned int) data->vma;
		}
		if (!strcmp(symbols[x]->name, ".leteral.app_main")) {
			printf("Changing address for symbol: %s\n", symbols[x]->name);
			symbols[x]->value = (long unsigned int) literal->vma;
		}
		if (!strcmp(symbols[x]->name, test_function_name)) {
			printf("Storing address of symbol: %s!\n", symbols[x]->name);
			test_function = text_start + symbols[x]->value;
		}
	}
	// test_function(in,out);
	relocate(literal);
	relocate(text);
	unsigned char  c;
	printf("data:\n");
    for(int x=0; x<data->size;x++){
        c = *((unsigned char*)data_start+x);
        printf("%c",c);
    }
    printf("\n");
	printf("literal:\n");
    for(int x=0; x<literal->size;x++){
        c = *((unsigned char*)literal_start+x);
        printf("0x%02x ",c);
    }
    printf("\n");
	printf("text:\n");
    for(int x=0; x<text->size;x++){
        c = *((unsigned char*)text_start+x);
        printf("0x%02x ",c);
    }
    printf("\n");
	free(symbols);
	bfd_close(abfd);
    return 0;
}

