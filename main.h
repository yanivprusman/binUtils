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
// #define INPUT_FILE_NAME "test_unit.o"
#define OUTPUT_FILE_NAME "/mnt/c/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main2.c.obj"
#define MAX_STRING_SIZE 1000
#define ALIGN 0x20
#define RODATA_SUBSTRING ".rodata."
#define LITERAL_SUBSTRING ".literal."
#define TEXT_SUBSTRING ".text."

bfd *inputFile,*outputFile;
asymbol **symbols;
int symCount;
uint32_t app_main,memoryStartEsp,nextMemoryAddressEsp;
uintptr_t rodataHostStart =0, literalHostStart = 0, textHostStart = 0;
uintptr_t rodataEspStart =0, literalEspStart = 0, textEspStart = 0;
uintptr_t memoryStart,nextMemoryAddress;
uint32_t addressOfPuts;
int myBreakpoint;
// uint32_t addressOfRodata = 0x11111100;
// uint32_t addressOfLiteral = 0x22222200;
// uint32_t addressOfText = 0x33333300;

/**
 * getMemorySize
 * allocate memory
 * align memoryStart
 * get address of memory alocated in esp
 * put sections in memory and set there vma to 
 ******************
 * memory layout:
 * memoryStart
 * allign
 * rodataSart= * rodata 1
 * allign
 * rodata n
 * allign
 * literalStart = literal 1
 * allign
 * literal n
 * allign
 * textStart = text 1 
 * 
 * 
 * 
 * 
 * 
 * 
 */











