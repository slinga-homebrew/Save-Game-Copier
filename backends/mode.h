#pragma once

#include "backend.h"

//
// MODE support contributed by Terraonion (https://github.com/Terraonion-dev)
//

bool modeIsBackupDeviceAvailable(int backupDevice);
int modeListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int modeReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int modeWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int modeDeleteSaveFile(int backupDevice, char* filename);

// helper functions
int modeEnter(void);
int modeExit(void);
int modeReadBUPHeader(char* filename, PBUP_HEADER bupHeader);
