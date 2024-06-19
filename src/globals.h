#ifndef GLOBALS_H
#define GLOBALS_H
#include "rom.h"

extern Rom rom;
extern char* programName;
extern char* outputFolder;
extern char* outputFilename;
extern char* compressionType;
extern char* patternSize;
extern char* patternDirection;
extern char* paletteDescription;
extern char* bitplaneType;
extern char* descriptorFilename;
extern char* checkRedundant;
extern int patternOverride;
extern int paletteOverride;
extern int bitplaneOverride;
extern int checkRedundantOverride;

#endif