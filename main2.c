#include <bfd.h>
#include <stdio.h>
#include <string.h>

#define FILE_NAME "/mnt/c/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define RODATA_NAME ".rodata.app_main.str1.4"
#define LITERAL_NAME ".literal.app_main"
#define TEXT_NAME ".text.app_main"

int main() {
    // Initialize BFD
    bfd_init();
    bfd *abfd = bfd_openr(FILE_NAME, NULL);
    if (!abfd) {
        bfd_perror("Failed to open file");
        return 1;
    }

    if (!bfd_check_format(abfd, bfd_object)) {
        bfd_perror("File format not recognized as an object file");
        return 1;
    }

    asection *sec;
    unsigned long rodata_size = 0, text_size = 0, literal_size = 0;

    for (sec = abfd->sections; sec != NULL; sec = sec->next) {
        const char *name = bfd_section_name(sec);

        if (strcmp(name, RODATA_NAME) == 0) {
            rodata_size = bfd_section_size(sec);
        } else if (strcmp(name, TEXT_NAME) == 0) {
            text_size = bfd_section_size(sec);
        } else if (strcmp(name, LITERAL_NAME) == 0) {
            literal_size = bfd_section_size(sec);
        }
    }

    printf("Sizes:\n");
    printf(".rodata: %lu bytes\n", rodata_size);
    printf(".text: %lu bytes\n", text_size);
    printf(".literal: %lu bytes\n", literal_size);

    unsigned long rodata_addr, text_addr, literal_addr;

    printf("Enter desired memory address for .rodata: ");
    scanf("%lx", &rodata_addr);

    printf("Enter desired memory address for .text: ");
    scanf("%lx", &text_addr);

    printf("Enter desired memory address for .literal: ");
    scanf("%lx", &literal_addr);

    // Relocate sections
    for (sec = abfd->sections; sec != NULL; sec = sec->next) {
        const char *name = bfd_section_name(sec);

        if (strcmp(name, RODATA_NAME) == 0) {
            bfd_set_section_vma(sec, rodata_addr);
        } else if (strcmp(name, TEXT_NAME) == 0) {
            bfd_set_section_vma(sec, text_addr);
        } else if (strcmp(name, LITERAL_NAME) == 0) {
            bfd_set_section_vma(sec, literal_addr);
        }
    }

    // Write relocated file
    bfd *outbfd = bfd_openw("relocated_main.c.obj", bfd_get_target(abfd));
    if (!outbfd) {
        bfd_perror("Failed to create output file");
        return 1;
    }

    bfd_set_format(outbfd, bfd_get_format(abfd));
    if (!bfd_copy_private_header_data(abfd, outbfd)) {
        bfd_perror("Failed to copy private header data");
        return 1;
    }

    // for (sec = abfd->sections; sec != NULL; sec = sec->next) {
    //     if (!bfd_copy_private_section_data(abfd, sec, outbfd)) {
    //         bfd_perror("Failed to copy private section data");
    //         return 1;
    //     }
    // }
    for (sec = abfd->sections; sec != NULL; sec = sec->next) {
        // Create a new section in the output BFD corresponding to the input section
        asection *outsec = bfd_make_section_anyway(outbfd, bfd_section_name(sec));
        if (!outsec) {
            bfd_perror("Failed to create section in output file");
            return 1;
        }

        // Copy the section-specific data
        if (!bfd_copy_private_section_data(abfd, sec, outbfd, outsec)) {
            bfd_perror("Failed to copy private section data");
            return 1;
        }
    }

    bfd_close(outbfd);
    bfd_close(abfd);

    return 0;
}
