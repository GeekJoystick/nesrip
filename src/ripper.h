#ifndef RIPPER_H
#define RIPPER_H
#include "rom.h"

#define TILESHEET_WIDTH 16 * 8
#define TILESHEET_HEIGHT 16 * 8
#define MAX_PATTERN_SIZE 8

int ripSection(Rom* rom, char* sectionStartString, char* sectionEndString, char* patternSizeString, char* paletteDescription, char* compressionType, char* filePrefix, char* filenameOverload);

#endif