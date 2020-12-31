#pragma once

#include "backup.h"

bool cdIsBackupDeviceAvailable(int backupDevice);
int cdListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int cdReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);

// helper functions
int cdReadBUPHeader(char* filename, PBUP_HEADER bupHeader);

