#include <stdio.h>
#include <malloc.h>
#include "globals.h"
#include "logger.h"
#include "ripper.h"
#include "utils.h"
#include "sha_2/sha-256.h"

char* hashString;
char* tempHashString;
int foundRom = false;

#define CHECK_COMMAND(command, callback) \
if (strcmp(token, command) == 0) \
{ \
	if (callback()) \
		break; \
	\
	token = updateToken(NULL); \
	continue; \
}

#define PULL_TOKEN(command, token) \
token = updateToken(NULL); \
\
if (token == NULL) \
{ \
	printInvalidDatabaseCommand(command, "File ended"); \
	return 1; \
}

char* updateToken(char* string)
{
	char* token = strtok(string, " \n");
	return token;
}

int removeCarriageReturns(char* string, int length)
{
	int hits = 0;
	char* carriageReturn = strchr(string, 13);

	while (carriageReturn != NULL)
	{
		memcpy(carriageReturn, carriageReturn + 1, strlen(carriageReturn + 1));
		carriageReturn = strchr(string, 13);
		hits++;
	}

	memset(string + length - hits - 1, 0, hits);
	return length - hits;
}

int handleHashCommand()
{
	char* token;
	PULL_TOKEN("Hash", token);

	if (strlen(token) < SIZE_OF_SHA_256_HASH * 2)
	{
		printf("Warning: Matching hash \"");
		printf(token);
		printf("\"  is too small.\n");
		return 0;
	}

	memcpy(tempHashString, token, SIZE_OF_SHA_256_HASH * 2);
	tempHashString[SIZE_OF_SHA_256_HASH * 2] = 0;

	strupr(tempHashString);

	if (strcmp(hashString, tempHashString) == 0)
	{
		printf("Matched hash!\n");
		foundRom = true;
	}

	return 0;
}

int handlePatternCommand()
{
	char* size, * direction;
	PULL_TOKEN("Pattern", size);
	PULL_TOKEN("Pattern", direction);

	if (patternOverride)
		return 0;

	patternSize = size;
	patternDirection = direction;

	return 0;
}

int handlePaletteCommand()
{
	char* token;
	PULL_TOKEN("Palette", token);

	if (paletteOverride)
		return 0;

	paletteDescription = token;
	return 0;
}

int handleSectionCommand()
{
	char* prefix, * sectionStart, * sectionEnd;

	PULL_TOKEN("Section", prefix);
	PULL_TOKEN("Section", sectionStart);
	PULL_TOKEN("Section", sectionEnd);

	ExtractionArguments args =
	{
		sectionStart,
		sectionEnd,
		patternSize,
		patternDirection,
		paletteDescription,
		compressionType,
		bitplaneType,
		checkRedundant,
		outputFolder,
		prefix
	};

	ripSection(&rom, &args);
	return 0;
}

int handleCompressionCommand()
{
	char* token;
	PULL_TOKEN("Compression", token);

	compressionType = token;
	return 0;
}

int handleBitplaneCommand()
{
	char* token;
	PULL_TOKEN("Bitplane", token);

	if (bitplaneOverride)
		return 0;

	bitplaneType = token;
	return 0;
}

int handleCheckRedundantCommand()
{
	char* token;
	PULL_TOKEN("CheckRedundant", token);

	if (checkRedundantOverride)
		return 0;

	checkRedundant = token;
	return 0;
}

int handleClearRedundantCommand()
{
	cleanupPatternChains();
	initPatternChains();
	return 0;
}

void interpretDatabase()
{
	initPatternChains();

	uint8_t hash[SIZE_OF_SHA_256_HASH];
	calc_sha_256(hash, rom.data, rom.size);

	hashString = (char*)malloc(sizeof(char) * SIZE_OF_SHA_256_HASH * 2 + 1);
	tempHashString = (char*)malloc(sizeof(char) * SIZE_OF_SHA_256_HASH * 2 + 1);

	if (hashString == NULL)
	{
		printf("Error: Couldn't allocate memory for ROM hash string.\n");
		return;
	}

	char* hashStringPtr = hashString;

	for (int i = 0; i < SIZE_OF_SHA_256_HASH; i++)
	{
		sprintf(hashStringPtr, "%02X", hash[i]);
		hashStringPtr += 2;
	}
	hashStringPtr[0] = 0;

	printf("ROM Hash: ");
	printf(hashString);
	printf("\n");

	char* database;
	int databaseLength = readAllBytesFromFile(descriptorFilename, &database, true);
	foundRom = false;

	databaseLength = removeCarriageReturns(database, databaseLength);

	char* token = updateToken(database);
	while (token != NULL)
	{
		if (!foundRom)
		{
			if (strcmp(token, "hash") == 0)
			{
				if (handleHashCommand())
					break;
			}

			token = updateToken(NULL);
			continue;
		}

		CHECK_COMMAND("p", handlePatternCommand);
		CHECK_COMMAND("i", handlePaletteCommand);
		CHECK_COMMAND("c", handleCompressionCommand);
		CHECK_COMMAND("b", handleBitplaneCommand);
		CHECK_COMMAND("r", handleCheckRedundantCommand);
		CHECK_COMMAND("k", handleClearRedundantCommand);
		CHECK_COMMAND("s", handleSectionCommand);

		if (strcmp(token, "end") == 0)
			break;

		printf("Invalid database token: ");
		printf(token);
		printf("\n");
		break;
	}

	if (!foundRom)
		printf("Error: Could not match ROM with database.\n");

	cleanupPatternChains();
	free(hashString);
	free(tempHashString);
}
