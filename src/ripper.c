#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "logger.h"
#include "utils.h"
#include "ripper.h"

char paletteData[] = {
	0, 0, 0, 255,
	239, 239, 239, 255,
	222, 123, 82, 255,
	41, 115, 156, 255
};

typedef struct{
	Rom* rom;
	ExtractionArguments* args;
	int sectionStart;
	int sectionEnd;
	int patternSize;
	int patternDirection;

	char* sheet;
	int tx;
	int ty;
	int stx;
	int sty;
	int maxX;
	int maxY;
} ExtractionContext;

int patternHorizontal = false;

int allocTilesheet(ExtractionContext* context, int tileCount) {
	int width = 128;
	int height = (tileCount / (16 * context->patternSize)) * context->patternSize * 8;

	if (tileCount % (16 * context->patternSize) != 0){
		height += context->patternSize * 8;
	}

	context->sheet = (char*)malloc(width * height * 4);

	if (context->sheet == NULL) {
		printf("Error: Couldn't allocate memory for tilesheet.\n");
		return 1;
	}

	memset(context->sheet, 0, width * height * 4);
	return 0;
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
		sheet[(y * 128 + x) * 4 + i] = color[i];
	}
}

char* allocOverloadedFilename(ExtractionContext* context) {
	ExtractionArguments* args = context->args;
	int lenOutputFolder = strlen(args->outputFolder);
	int lenFilename = strlen(args->filenameOverload);
	int lenPaletteDescription = strlen(args->paletteDescription);

	char* result = (char*)malloc(lenOutputFolder + lenFilename + lenPaletteDescription + 6);

	if (result == NULL) {
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}

	char* outputFilenamePtr = result;

	memcpy(outputFilenamePtr, args->outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, args->filenameOverload, lenFilename);
	outputFilenamePtr += lenFilename;
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, args->paletteDescription, lenPaletteDescription);
	outputFilenamePtr += lenPaletteDescription;
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;

	return result;
}

char* allocSectionFilename(ExtractionContext* context) {
	ExtractionArguments* args = context->args;
	int lenOutputFolder = strlen(args->outputFolder);
	int lenSectionStart = strlen(args->sectionStartString);
	int lenSectionEnd = strlen(args->sectionEndString);
	int lenPaletteDescription = strlen(args->paletteDescription);

	char* result = (char*)malloc(lenOutputFolder + lenSectionStart + lenSectionEnd + lenPaletteDescription + 9);

	if (result == NULL) {
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}

	char* outputFilenamePtr = result;

	memcpy(outputFilenamePtr, args->outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, args->sectionStartString, lenSectionStart);
	outputFilenamePtr += lenSectionStart;
	*(outputFilenamePtr++) = '_';
	memcpy(outputFilenamePtr, args->sectionEndString, lenSectionEnd);
	outputFilenamePtr += lenSectionEnd;
	*(outputFilenamePtr++) = '_';
	*(outputFilenamePtr++) = args->patternDirectionString[0];
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, args->paletteDescription, lenPaletteDescription);
	outputFilenamePtr += lenPaletteDescription;
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;

	return result;
}

int writeOutput(char* outputData, int width, int height, ExtractionContext* context) {
	char* outputFilename = NULL;

	if (context->args->filenameOverload != NULL) {
		outputFilename = allocOverloadedFilename(context);
	}
	else {
		outputFilename = allocSectionFilename(context);
	}

	if (outputFilename == NULL) {
		return 0;
	}

	printf("  Writing sheet to \"");
	printf(outputFilename);
	printf("\".\n");

	if (!stbi_write_png(outputFilename, width, height, 4, outputData, 128 * 4)) {
		printf("An error occurred while writing to output file ");
		printf(outputFilename);
		printf(".\n");
		return 0;
	}

	free(outputFilename);

	return 1;
}

int getSectionDetails(Rom* rom, ExtractionContext* context) {
	ExtractionArguments* args = context->args;

	if (str2int(&(context->sectionStart), args->sectionStartString, 16) != STR2INT_SUCCESS || context->sectionStart < 0 || context->sectionStart >= rom->size) {
		printf("Error: Invalid section start address.\n");
		return 0;
	}

	if (str2int(&(context->sectionEnd), args->sectionEndString, 16) != STR2INT_SUCCESS || context->sectionEnd < 0 || context->sectionEnd >= rom->size) {
		printf("Error: Invalid section end address.\n");
		return 0;
	}

	if (str2int(&(context->patternSize), args->patternSizeString, 10) != STR2INT_SUCCESS || numberOfSetBits(context->patternSize) > 1 || (unsigned int)context->patternSize > MAX_PATTERN_SIZE) {
		printf("Error: Invalid pattern size.\n");
		return 0;
	}

	if (context->sectionEnd < context->sectionStart) {
		printf("Error: Section end is placed before start.\n");
		return 0;
	}

	if (strcmp(args->patternDirectionString, "h") == 0) {
		context->patternDirection = false;
	}
	else if (strcmp(args->patternDirectionString, "v") == 0) {
		context->patternDirection = true;
	}
	else {
		printf("Error: Invalid pattern direction. Use \"h\" or \"v\".\n");
	}

	return 1;
}

void writeLine(ExtractionContext* context, int y, char low, char high){
	int stx = (context->patternDirection) ? context->sty : context->stx;
	int sty = (context->patternDirection) ? context->stx : context->sty;

	int px;
	int py = (context->ty * context->patternSize + sty) * 8 + y;

	for (int x = 0; x < 8; x++) {
		char c = ((high & 1) << 1) | (low & 1);
		high >>= 1;
		low >>= 1;

		px = (context->tx * context->patternSize + stx) * 8 + 7 - x;
		drawPixel(context->sheet, px, py, getColor(c, context->args->paletteDescription));
	}

	if (px + 7 > context->maxX){
		context->maxX = px + 7;
	}

	if (py > context->maxY){
		context->maxY = py;
	}
}

void incrementTilePos(ExtractionContext* context){
	context->stx++;
	if (context->stx >= context->patternSize) {
		context->stx = 0;
		context->sty++;

		if (context->sty >= context->patternSize) {
			context->sty = 0;

			context->tx++;
			if (context->tx >= 16 / context->patternSize) {
				context->tx = 0;
				context->ty++;
			}
		}
	}
}

int ripSectionRaw(Rom* rom, ExtractionContext* context) {
	printf("Ripping raw section from ");
	printf(context->args->sectionStartString);
	printf(" to ");
	printf(context->args->sectionEndString);
	printf(" in ROM.\n");

	if (!getSectionDetails(rom, context)) {
		return 0;
	}

	if (((context->sectionEnd - context->sectionStart) + 1) % 16 != 0) {
		printf("Warning: Targeted section has some extra bytes that cannot be used to make a full tile.\n         Rounding down section end address.\n");
		context->sectionEnd -= (context->sectionEnd - context->sectionStart + 1) % 16;
	}

	if (allocTilesheet(context, (context->sectionEnd - context->sectionStart + 1) / 16)){
		return 0;
	}

	if (context->sheet == NULL) {
		return 0;
	}

	int sheetCount = 0;
	int tx = 0;
	int ty = 0;
	int stx = 0;
	int sty = 0;

	char* sectionData = rom->data + context->sectionStart;
	char* endPointer = rom->data + context->sectionEnd;

	while (sectionData < endPointer) {
		for (int y = 0; y < 8; y++) {
			char low = sectionData[y];
			char high = sectionData[y + 8];
			writeLine(context, y, low, high);
		}

		incrementTilePos(context);
		sectionData += 16;
	}

	writeOutput(context->sheet, context->maxX + 1, context->maxY + 1, context);
	free(context->sheet);

	return 1;
}

int ripSection(Rom* rom, ExtractionArguments* arguments)
{
	ExtractionContext context = {
		rom,
		arguments
	};

	if (strcmp(arguments->compressionType, "raw") == 0) {
		if (!ripSectionRaw(rom, &context)) {
			printf("An error occured during ripping.\n");
			return 0;
		}
	}
	else {
		printf("Error: Unknown compression type \"");
		printf(arguments->compressionType);
		printf("\".\n");
		return 0;
	}

	return 1;
}