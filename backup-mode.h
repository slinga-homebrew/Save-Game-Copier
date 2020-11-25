#pragma once

#include "backup.h"

bool modeIsBackupDeviceAvailable(int backupDevice);
int modeListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int modeReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int modeWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int modeDeleteSaveFile(int backupDevice, char* filename);

// helper functions
int modeEnter();
int modeExit();
