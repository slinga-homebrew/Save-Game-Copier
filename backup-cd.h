#pragma once

#include "backup.h"

bool cdIsBackupDeviceAvailable(int backupDevice);
int cdListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int cdReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);

// helper functions
void cdToBackupName(char* input, char* output);
void backupNameToCDName(char* input, char* output);
