#include "bfd.h"
#include <stdio.h>
#include <stdlib.h>
const char *inputFileName = "/mnt/c//101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj";
const char *outputFileName = "/mnt/c//101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/output.bin";
bfd *inputFile,*outputFile;
void doInit(){
    bfd_init();
    inputFile = bfd_openr(inputFileName, NULL);
    if (!inputFile) {
        printf("Failed to open input BFD file\n");
        return;
    }
    if (!bfd_check_format(inputFile, bfd_object)) {
        printf("Input file is not a valid object file\n");
        bfd_close(inputFile);
        return;
    }
    const char* getTarget =bfd_get_target(inputFile);
    outputFile = bfd_openw(outputFileName,getTarget);
    if (!outputFile) {
        bfd_perror("Failed to create output BFD file");
        bfd_close(inputFile);
        return;
    }
    if (!bfd_set_format(outputFile, bfd_object)) {
        bfd_perror("Failed to set format");
    }
}

void doCopy(){
    char*s=malloc(1000);
    asection* section,*newSection;
    for(section=inputFile->sections;section!=NULL;section=section->next){
        // printf("%s\n",section->name);
        newSection = bfd_make_section_anyway(outputFile, section->name);
        if (!newSection) {
            sprintf(s,"Failed to create section %s in output file",section->name);
            bfd_perror(s);
            continue;
        }
        if (!bfd_set_section_size(newSection, bfd_section_size(section))) {
            bfd_perror("Failed to set section size");
            continue;
        }
        if (!bfd_set_section_vma(newSection, section->vma)) {
            bfd_perror("Failed to set section VMA");
            continue;
        }
        if (section->size > 0) {
            void *sectionData = malloc(section->size);
            if (!bfd_get_section_contents(inputFile, section, sectionData, 0, section->size)) {
                bfd_perror("Failed to get section contents");
                free(sectionData);
                continue;
            }
            if (!bfd_set_section_contents(outputFile, newSection, sectionData, 0, section->size)) {
                bfd_perror("Failed to write section contents");
                free(sectionData);
                continue;
            }
            free(sectionData);
        }
    }
}
void main() {
    doInit();
    doCopy();
    // char*sectionName=".rodata.app_main.str1.4";
    // asection *section = bfd_get_section_by_name(inputFile,sectionName);
    if(!bfd_close(outputFile)){
        bfd_perror("closing");
    }
    return;
}
