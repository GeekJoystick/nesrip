#include <stdio.h>
#include <malloc.h>
#include "rom.h"
#include "utils.h"

Rom readRom(char* romName) {
    Rom result = {(char*)NULL, -1};

    result.size = readAllBytesFromFile(romName, &result.data, false);

    if (result.size < 0) {
        return result;
    }
    
    if (result.size < 4) {
        printf("Error: File is not a NES ROM.\n");
        free(result.data);

        result.data = (char*)NULL;
        result.size = -1;
        return result;
    }

    if (result.data[0] != 'N' || result.data[1] != 'E' || result.data[2] != 'S' || result.data[3] != 0x1A) {
        printf("Error: File is not a NES ROM.\n");
        free(result.data);

        result.data = (char*)NULL;
        result.size = -1;
        return result;
    }
    
    return result;
}

void freeRom(Rom* rom) {
    if (rom->data == NULL) return;
    free(rom->data);
}