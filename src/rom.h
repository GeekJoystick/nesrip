#ifndef ROM_H
#define ROM_H

typedef struct {
	char* originalData;
	char* data;
	int size;
} Rom;

Rom readRom(char* romName);
void freeRom(Rom* rom);

#endif