#pragma once

#include "backend.h"


bool modemIsBackupDeviceAvailable(int backupDevice);
int modemListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int modemReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int modemWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int modemDeleteSaveFile(int backupDevice, char* filename);
