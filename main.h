#ifndef PACKAGE
#define PACKAGE "espBfd"
#define PACKAGE_VERSION "0.1"
#endif
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "runtime-static-linking-with-libbfd2.c"
#define INPUT_FILE_NAME "/mnt/c/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define OUTPUT_FILE_NAME "/mnt/c/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main2.c.obj"
#define MAX_STRING_SIZE 1000
#define ADDRESS_OF_RODATA 0x11111100
#define ADDRESS_OF_LITERAL 0x22222200
#define ADDRESS_OF_TEXT 0x33333300
#define ADDRESS_OF_PUTS 0x44444444
#define ALLIGN 0x20
#define RODATA_SUBSTRING ".rodata."
#define LITERAL_SUBSTRING ".literal."
#define TEXT_SUBSTRING ".text."

bfd *inputFile,*outputFile;
asymbol **symbols;
int symCount;
uint32_t app_main;
uintptr_t rodataStart =0, literalStart = 0, textStart = 0;
uintptr_t memoryStart;












