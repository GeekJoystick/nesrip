#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "logger.h"
#include "utils.h"
#include "ripper.h"
#include "globals.h"

char paletteData[] = {
	0, 0, 0, 255,
	239, 239, 239, 255,
	222, 123, 82, 255,
	41, 115, 156, 255
};

void clearTilesheet(char* sheet) {
	memset(sheet, 0, TILESHEET_WIDTH * TILESHEET_HEIGHT * 4);
}

char* allocTilesheet() {
	char* result = (char*)malloc(TILESHEET_WIDTH * TILESHEET_HEIGHT * 4);

	if (result == NULL) {
		printf("Error: Couldn't allocate memory for tilesheet.\n");
		return NULL;
	}

	clearTilesheet(result);
	return result;
}

char* getColor(char color, char* paletteDescription) {
	char c = paletteDescription[color];

	switch (c) {
	case 'b':
		return paletteData;
	case 'w':
		return paletteData + 4;
	case 'o':
		return paletteData + 8;
	case 't':
		return paletteData + 12;
	}
	
	return paletteData;
}

void drawPixel(char* sheet, int x, int y, char* color) {
	for (int i = 0; i < 4; i++) {
		sheet[(y * TILESHEET_WIDTH + x) * 4 + i] = color[i];
	}
}

char* allocOverloadedFilename(char* filename, int suffix, char* paletteDescription) {
	int lenOutputFolder = strlen(outputFolder);
	int lenFilename = strlen(filename);
	int lenPaletteDescription = strlen(paletteDescription);
	int lenFileSuffix = numDigits(suffix) + 1;

	char* result = (char*)malloc(lenOutputFolder + lenFilename + lenFileSuffix + lenPaletteDescription + 6);

	if (result == NULL) {
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}

	char* outputFilenamePtr = result;

	memcpy(outputFilenamePtr, outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, filename, lenFilename);
	outputFilenamePtr += lenFilename;
	sprintf(outputFilenamePtr, "-%d", suffix);
	outputFilenamePtr += lenFileSuffix;
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, paletteDescription, lenPaletteDescription);
	outputFilenamePtr += lenPaletteDescription;
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;

	return result;
}

char* allocSectionFilename(char* prefix, char* sectionStart, char* sectionEnd, int suffix, char* paletteDescription) {
	int lenOutputFolder = strlen(outputFolder);
	int lenFilePrefix = strlen(prefix);
	int lenSectionStart = strlen(sectionStart);
	int lenSectionEnd = strlen(sectionEnd);
	int lenPaletteDescription = strlen(paletteDescription);
	int lenFileSuffix = numDigits(suffix) + 1;

	char* result = (char*)malloc(lenOutputFolder + lenSectionStart + lenSectionEnd + lenFilePrefix + lenFileSuffix + lenPaletteDescription + 7);

	if (result == NULL) {
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}

	char* outputFilenamePtr = result;

	memcpy(outputFilenamePtr, outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, prefix, lenFilePrefix);
	outputFilenamePtr += lenFilePrefix;
	memcpy(outputFilenamePtr, sectionStart, lenSectionStart);
	outputFilenamePtr += lenSectionStart;
	*(outputFilenamePtr++) = '_';
	memcpy(outputFilenamePtr, sectionEnd, lenSectionEnd);
	outputFilenamePtr += lenSectionEnd;
	sprintf(outputFilenamePtr, "-%d", suffix);
	outputFilenamePtr += lenFileSuffix;
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, paletteDescription, lenPaletteDescription);
	outputFilenamePtr += lenPaletteDescription;
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;

	return result;
}

int writeOutput(char* outputData, int width, int height, char* sectionStartString, char* sectionEndString, char* patternSizeString, char* filePrefix, int fileSuffix, char* paletteDescription, char* filenameOverload) {
	char* outputFilename = NULL;

	if (filenameOverload != NULL) {
		outputFilename = allocOverloadedFilename(filenameOverload, fileSuffix, paletteDescription);
	}
	else {
		outputFilename = allocSectionFilename(filePrefix, sectionStartString, sectionEndString, fileSuffix, paletteDescription);
	}

	if (outputFilename == NULL) {
		return 0;
	}

	printf("  Writing sheet to \"");
	printf(outputFilename);
	printf("\".\n");

	if (!stbi_write_png(outputFilename, width, height, 4, outputData, TILESHEET_WIDTH * 4)) {
		printf("An error occurred while writing to output file ");
		printf(outputFilename);
		printf(".\n");
		return 0;
	}

	free(outputFilename);

	return 1;
}

int getSectionDetails(Rom* rom, char* sectionStartString, char* sectionEndString, char* patternSizeString, int* sectionStart, int* sectionEnd, int* patternSize) {

	if (str2int(sectionStart, sectionStartString, 16) != STR2INT_SUCCESS || *sectionStart < 0 || *sectionStart >= rom->size) {
		printf("Error: Invalid section start address.\n");
		return 0;
	}

	if (str2int(sectionEnd, sectionEndString, 16) != STR2INT_SUCCESS || *sectionEnd < 0 || *sectionEnd >= rom->size) {
		printf("Error: Invalid section end address.\n");
		return 0;
	}

	if (str2int(patternSize, patternSizeString, 10) != STR2INT_SUCCESS || numberOfSetBits(*patternSize) > 1 || (unsigned int)*patternSize > MAX_PATTERN_SIZE) {
		printf("Error: Invalid pattern size.\n");
		return 0;
	}

	if (*sectionEnd < *sectionStart) {
		printf("Error: Section end is placed before start.\n");
		return 0;
	}

	return 1;
}

int ripSectionRaw(Rom* rom, char* sectionStartString, char* sectionEndString, char* patternSizeString, char* paletteDescription, char* filePrefix, char* filenameOverload) {
	printf("Ripping raw section from ");
	printf(sectionStartString);
	printf(" to ");
	printf(sectionEndString);
	printf(" in ROM.\n");

	int sectionStart = 0;
	int sectionEnd = 0;
	int patternSize = 0;

	if (!getSectionDetails(rom, sectionStartString, sectionEndString, patternSizeString, &sectionStart, &sectionEnd, &patternSize)) {
		return 0;
	}

	char* sheet = allocTilesheet();

	if (sheet == NULL) {
		return 0;
	}

	if ((sectionEnd - sectionStart + 1) % 16 != 0) {
		printf("Warning: Targeted section has some extra bytes that cannot be used to make a full tile.\n         Rounding down section end address.\n");
		sectionEnd -= (sectionEnd - sectionStart + 1) % 16;
	}

	int sheetCount = 0;
	int tx = 0;
	int ty = 0;
	int stx = 0;
	int sty = 0;

	int maxX = 0;
	int maxY = 0;

	char* sectionData = rom->data + sectionStart;
	char* endPointer = rom->data + sectionEnd;

	while (sectionData < endPointer) {
		for (int y = 0; y < 8; y++) {
			char low = sectionData[y];
			char high = sectionData[y + 8];

			for (int x = 0; x < 8; x++) {
				char c = ((high & 1) << 1) | (low & 1);
				high >>= 1;
				low >>= 1;

				int px = (tx * patternSize + stx) * 8 + 7 - x;
				int py = (ty * patternSize + sty) * 8 + y;

				if (px > maxX) {
					maxX = px;
				}

				if (py > maxY) {
					maxY = py;
				}

				drawPixel(sheet, px, py, getColor(c, paletteDescription));
			}
		}

		stx++;
		if (stx >= patternSize) {
			stx = 0;
			sty++;

			if (sty >= patternSize) {
				sty = 0;

				tx++;
				if (tx >= 16 / patternSize) {
					tx = 0;
					ty++;
					if (ty >= 16 / patternSize) {
						ty = 0;

						writeOutput(sheet, TILESHEET_WIDTH, TILESHEET_HEIGHT, sectionStartString, sectionEndString, patternSizeString, filePrefix, sheetCount++, paletteDescription, filenameOverload);
						clearTilesheet(sheet);
					}
				}
			}
		}

		
		sectionData += 16;
	}

	if (stx + sty + tx + ty > 0) {
		writeOutput(sheet, maxX + 1, maxY + 1, sectionStartString, sectionEndString, patternSizeString, filePrefix, sheetCount, paletteDescription, filenameOverload);
	}

	free(sheet);

	return 1;
}

int ripSection(Rom* rom, char* sectionStartString, char* sectionEndString, char* patternSizeString, char* paletteDescription, char* compressionType, char* filePrefix, char* filenameOverload)
{
	if (strcmp(compressionType, "raw") == 0) {
		if (!ripSectionRaw(rom, sectionStartString, sectionEndString, patternSizeString, paletteDescription, filePrefix, filenameOverload)) {
			printf("An error occured during ripping.");
			return 0;
		}
	}
	else {
		printf("Error: Unknown compression type \"");
		printf(compressionType);
		printf("\".\n");
		return 0;
	}

	return 1;
}