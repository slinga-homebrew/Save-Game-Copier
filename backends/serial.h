#pragma once

#include "backend.h"


bool serialIsBackupDeviceAvailable(int backupDevice);
int serialListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int serialReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int serialWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int serialDeleteSaveFile(int backupDevice, char* filename);
