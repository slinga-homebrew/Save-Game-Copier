#pragma once

#include "backup.h"

int saturnListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves);
int saturnReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize);
int saturnWriteSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize);
int saturnDeleteSaveFile(int backupDevice, char* filename);
int saturnFormatDevice(int backupDevice);
