#pragma once

#include "backup.h"

int satiatorListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int satiatorReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int satiatorWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int satiatorDeleteSaveFile(int backupDevice, char* filename);

// helper functions
int satiatorEnter();
int satiatorExit();
