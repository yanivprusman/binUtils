#define PACKAGE "circe-dynload-test"
#define PACKAGE_VERSION "0.1"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <bfd.h>
#define INPUT_FILE_NAME "/mnt/c//101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define SYMBOL_NAME  "app_main"
#define TEXT_SECTION ".text.app_main"
#define DATA_SECTION ".rodata.app_main.str1.4"
int my_callback_01(int a) {
	printf("my_callback_01 called!\n");
	return a*2;
}

int myCallback2(int a) {
	printf("myCallback2 called!\n");
	return a*4;
}

typedef int (*callback_t)(int);
typedef void (*testFunction_t)(int, int*);
callback_t my_callback = myCallback2;
int in = 10;
int out[2] = {0,0};
testFunction_t test_function;
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

int main(int argc, char **argv) {
	bfd_init();
    printf("loading %s.\n",INPUT_FILE_NAME);
	bfd *abfd = bfd_openr(INPUT_FILE_NAME, NULL);
	if (!bfd_check_format (abfd, bfd_object)) {
		printf("Failed to open object file!\n");
		exit(-1);        
    }
	asection *text = bfd_get_section_by_name (abfd, TEXT_SECTION);
	asection *data = bfd_get_section_by_name (abfd, DATA_SECTION);
	size_t minMemSize = data->size + text->size;
	alloc_rwx(minMemSize);
	bfd_get_section_contents(abfd, data, memory, 0, data->size);
	bfd_get_section_contents(abfd, text, memory + data->size, 0, text->size);
	void *data_start = memory;
	void *text_start = memory + data->size;
	text->output_offset = (long unsigned int) text_start;
	data->output_offset = (long unsigned int) data_start;
	size_t symsize = bfd_get_symtab_upper_bound(abfd);
	asymbol **symbols = malloc(symsize);
	int symcount = bfd_canonicalize_symtab(abfd, symbols);
	int i;
	for (i = 0; i<symcount; i++) {
		if (!strcmp(symbols[i]->name, "callback")) {
			printf("Changing address for undefined symbol: %s\n", symbols[i]->name);
			symbols[i]->value = (long unsigned int) my_callback;
		}
		if (!strcmp(symbols[i]->name, SYMBOL_NAME)) {
			printf("Storing address of symbol: %s!\n", symbols[i]->name);
			test_function = text_start + symbols[i]->value;
		}
	}
	long relsize = bfd_get_reloc_upper_bound (abfd, text);
	arelent **relpp = malloc (relsize);
	long relcount = bfd_canonicalize_reloc (abfd, data, relpp, symbols);
	// long relcount = bfd_canonicalize_reloc (abfd, text, relpp, symbols);
	char *tmp;
    bfd_reloc_status_type type;
	for (i = 0; i<relcount; i++) {
		arelent *reloc = relpp[i];
		if (!strcmp((*reloc->sym_ptr_ptr)->name, ".literal.app_main")) {
            (*reloc->sym_ptr_ptr)->value = 0xdddddddd;
			printf("Found relocation for symbol: %s!\n", (*reloc->sym_ptr_ptr)->name);
			text->output_section = (*reloc->sym_ptr_ptr)->section;
            type = bfd_perform_relocation(abfd, reloc, text_start, text, NULL, &tmp);
		}
	}
	// test_function(in,out);
    FILE *f = fopen("output.bin","w+");
    fwrite(test_function,text->size,1,f);
    unsigned char  c;
    for(int x=0; x<text->size;x++){
        c = *((unsigned char*)text_start+x);
        printf("0x%02x ",c);
    }
    printf("\n");
	free(symbols);
	free (relpp);
	bfd_close(abfd);
    return 0;
}

