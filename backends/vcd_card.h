#pragma once

#include "backend.h"

#define VCD_CARD_FIRMWARE_SIZE (512 * 1024)

bool vcdIsBackupDeviceAvailable(int backupDevice);
int vcdListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int vcdReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
