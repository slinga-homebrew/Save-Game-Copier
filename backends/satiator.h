#pragma once

#include "backup.h"

bool satiatorIsBackupDeviceAvailable(int backupDevice);
int satiatorListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int satiatorReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int satiatorWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int satiatorDeleteSaveFile(int backupDevice, char* filename);

// helper functions
int satiatorEnter(void);
int satiatorExit(void);
int satiatorReadBUPHeader(char* filename, PBUP_HEADER bupHeader);
void satiatorReboot(void);
