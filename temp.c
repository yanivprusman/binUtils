#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bfd.h>

#define INPUT_FILE_NAME "/mnt/c/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define OUTPUT_FILE_NAME "main.bin"

void error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

void read_elf_file(const char *filename) {
    bfd *abfd;
    bfd_init(); // Initialize BFD library

    abfd = bfd_openr(filename, NULL);
    if (!abfd) {
        error("Failed to open file with BFD");
    }

    if (!bfd_check_format(abfd, bfd_object)) {
        error("Not a valid ELF file");
    }

    // Iterate over sections
    for (sec_ptr section = abfd->sections; section; section = section->next) {
        const char *section_name = bfd_section_name(section);
        printf("Section: %s\n", section_name);

        // Allocate buffer for section contents
        void *section_data = malloc(section->size);
        if (!section_data) {
            error("Failed to allocate memory for section data");
        }

        // Load the section
        if (!bfd_get_section_contents(abfd, section, section_data, 0, section->size)) {
            fprintf(stderr, "Failed to read section contents for section: %s\n", section_name);
            free(section_data);
            continue;
        }

        // Process section contents
        printf("Section size: %ld\n", (long)section->size);

        // Free the buffer
        free(section_data);
    }

    // Handle symbols
    long symbol_count;
    long symbol_table_size;
    asymbol **symbol_table = NULL;

    // Read the symbol table
    symbol_count = bfd_read_minisymbols(abfd, FALSE, (void**)&symbol_table, &symbol_table_size);
    if (symbol_count < 0) {
        error("Failed to read symbol table");
    }

    if (symbol_count > 0) {
        // Process symbols
        for (long i = 0; i < symbol_count; i++) {
            printf("Symbol: %s\n", bfd_asymbol_name(symbol_table[i]));
        }
    } else {
        printf("No symbols found in the symbol table.\n");
    }

    bfd_close(abfd);
}

int main() {
    read_elf_file(INPUT_FILE_NAME);
    return 0;
}
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

void generate_executable(bfd *input_bfd, const char *output_filename) {
    bfd *output_bfd;
    asymbol **symbol_table;
    long num_symbols;
    long storage_needed;
    asection *section;

    // Open the output ELF file
    output_bfd = bfd_openw(output_filename, bfd_get_target(input_bfd));
    if (!output_bfd) {
        bfd_perror("bfd_openw");
        return;
    }

    // Copy architecture and flags
    bfd_set_format(output_bfd, bfd_get_format(input_bfd));
    bfd_set_arch_mach(output_bfd, bfd_get_arch(input_bfd), bfd_get_mach(input_bfd));
    bfd_set_start_address(output_bfd, bfd_get_start_address(input_bfd));

    // Read and canonicalize symbol table
    storage_needed = bfd_get_symtab_upper_bound(input_bfd);
    if (storage_needed <= 0) {
        bfd_perror("bfd_get_symtab_upper_bound");
        return;
    }

    symbol_table = (asymbol **) malloc(storage_needed);
    num_symbols = bfd_canonicalize_symtab(input_bfd, symbol_table);
    if (num_symbols < 0) {
        bfd_perror("bfd_canonicalize_symtab");
        return;
    }

    // Modify the value of the symbol (example: setting the address)
    for (long i = 0; i < num_symbols; i++) {
        if (strcmp(bfd_asymbol_name(symbol_table[i]), "my_symbol") == 0) {
            symbol_table[i]->value = 0x400080;  // Set your desired address
        }
    }

    // Iterate over sections to copy them into the output BFD
    for (section = input_bfd->sections; section != NULL; section = section->next) {
        if (!(section->flags & SEC_HAS_CONTENTS)) continue;

        // Allocate a corresponding section in the output BFD
        asection *new_section = bfd_make_section_anyway_with_flags(output_bfd, section->name, section->flags);
        bfd_set_section_size(output_bfd, new_section, bfd_section_size(input_bfd, section));
        bfd_set_section_vma(output_bfd, new_section, bfd_section_vma(input_bfd, section));

        // Write section contents to the output BFD
        bfd_copy_private_section_data(input_bfd, section, output_bfd, new_section);

        // Transfer the section contents
        bfd_get_section_contents(input_bfd, section, section->contents, 0, bfd_section_size(input_bfd, section));
        bfd_set_section_contents(output_bfd, new_section, section->contents, 0, bfd_section_size(input_bfd, section));
    }

    // Write the symbol table into the output BFD
    bfd_set_symtab(output_bfd, symbol_table, num_symbols);

    // Finalize writing and close the BFD
    if (!bfd_close(output_bfd)) {
        bfd_perror("bfd_close");
    }

    free(symbol_table);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input-elf> <output-elf>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    // Open the input ELF file
    bfd *input_bfd = bfd_openr(input_filename, NULL);
    if (!input_bfd) {
        bfd_perror("bfd_openr");
        return EXIT_FAILURE;
    }

    // Check if the input file is an object file (ELF)
    if (!bfd_check_format(input_bfd, bfd_object)) {
        fprintf(stderr, "Not an object file.\n");
        return EXIT_FAILURE;
    }

    // Generate the executable from the input ELF file
    generate_executable(input_bfd, output_filename);

    // Close the input BFD
    if (!bfd_close(input_bfd)) {
        bfd_perror("bfd_close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
