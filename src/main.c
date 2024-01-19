#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "args.h"
#include "globals.h"
#include "interpreter.h"
#include "logger.h"
#include "ripper.h"
#include "rom.h"
#include "utils.h"
#include "sha_2/sha-256.h"

Rom rom;
char* outputFilename = NULL;
char* compressionType = "raw";
char* patternSize = "1";
char* paletteDescription = "bwot";
char* descriptorFilename = "nes_gfxdb.txt";
int patternOverride = false;
int paletteOverride = false;

void quitProgram(int code) {
	freeRom(&rom);
	exit(code);
}

int main(int argc, char** argv){
	findProgramName(argv[0]);

	if (argc < 2) {
		printNoInput();
		return 0;
	}

	if (checkHelpArg(argv[1])) {
		return 0;
	}

	char* inputRomName = argv[1];
	rom = readRom(inputRomName);

	if (rom.size < 0) {
		printf("An error occured while opening input ROM file.\n");
		return 0;
	}

	if (argc > 2) {
		if (handleAdditionnalArgs(0, argc - 2, argv + 2)) {
			quitProgram(0);
		}
	}

	printf("Ensuring output folder exists.\n");
	CreateDirectoryA("output", 0);

	if (argc > 2) {
		if (handleAdditionnalArgs(1, argc - 2, argv + 2)) {
			quitProgram(0);
		}
	}

	//TODO: Handle ROM hash detection and graphics ripping here
	interpretDatabase();

	quitProgram(0);
}