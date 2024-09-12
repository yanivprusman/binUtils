#include <stdio.h>
#include <stdlib.h>
#include <bfd.h>
#define FILE_NAME "main.c.obj"
void get_relocation_entries(bfd *abfd, asection *section, asymbol **symbol_table) {
    long relocation_count;
    arelent **relocs;
    long storage_needed;
    bfd_size_type size;

    // Get the number of relocations in the section
    relocation_count = bfd_get_reloc_upper_bound(abfd, section);
    if (relocation_count < 0) {
        printf("Error getting relocation count.\n");
        return;
    } else if (relocation_count == 0) {
        printf("No relocations found for section: %s\n", section->name);
        return;
    }

    // Allocate memory for relocation entries
    relocs = (arelent **) malloc(relocation_count);
    if (relocs == NULL) {
        printf("Memory allocation error.\n");
        return;
    }

    // Canonicalize the relocation entries
    size = bfd_canonicalize_reloc(abfd, section, relocs, symbol_table);
    if (size < 0) {
        printf("Error canonicalizing relocations.\n");
        free(relocs);
        return;
    }

    // Process each relocation entry
    for (long i = 0; i < size; i++) {
        arelent *reloc = relocs[i];
        printf("Relocation at address: %lx, symbol: %s, addend: %lx\n",
               reloc->address,
               (*reloc->sym_ptr_ptr)->name,
               reloc->addend);
    }

    free(relocs);
}

int main(int argc, char **argv) {

    bfd *abfd;
    bfd_init();

    // Open the binary file
    abfd = bfd_openr(FILE_NAME, NULL);
    if (!abfd) {
        printf("Failed to open file: %s\n", FILE_NAME);
        return 1;
    }

    if (!bfd_check_format(abfd, bfd_object)) {
        printf("File format not recognized as an object file.\n");
        bfd_close(abfd);
        return 1;
    }

    // Get the upper bound on the symbol table size
    long symtab_storage = bfd_get_symtab_upper_bound(abfd);
    if (symtab_storage < 0) {
        printf("Error getting symbol table size.\n");
        bfd_close(abfd);
        return 1;
    }

    // Allocate memory for the symbol table
    asymbol **symbol_table = (asymbol **) malloc(symtab_storage);
    if (symbol_table == NULL) {
        printf("Memory allocation error.\n");
        bfd_close(abfd);
        return 1;
    }

    // Canonicalize the symbol table
    long num_symbols = bfd_canonicalize_symtab(abfd, symbol_table);
    if (num_symbols < 0) {
        printf("Error canonicalizing symbol table.\n");
        free(symbol_table);
        bfd_close(abfd);
        return 1;
    }

    // Iterate through sections and get relocation entries
    for (asection *section = abfd->sections; section != NULL; section = section->next) {
        printf("Section: %s\n", section->name);
        get_relocation_entries(abfd, section, symbol_table);
    }

    // Cleanup
    free(symbol_table);
    bfd_close(abfd);

    return 0;
}
