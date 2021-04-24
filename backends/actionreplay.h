#pragma once

#include "../backup.h"

//
// Action Replay Cartridge
//

#define CARTRIDGE_MEMORY                0x02000000
#define ACTION_REPLAY_MAGIC_OFFSET      0x50
#define ACTION_REPLACE_SAVES_OFFSET     0x20000
#define ACTION_REPLACE_SAVES_SIZE       0x60000 // BUGBUG: just guessing here
#define ACTION_REPLAY_MAGIC             "ACTION REPLAY"
#define ACTION_REPLAY_PARTITION_SIZE    64

#define RLE01_MAGIC                     "RLE01"

bool actionReplayIsBackupDeviceAvailable(int backupDevice);
int actionReplayListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int actionReplayReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int actionReplayWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int actionReplayDeleteSaveFile(int backupDevice, char* filename);

// utility functions
int decompressRLE01(unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int* bytesNeeded);
int parseUncompressedMemory(unsigned char* arpUncompressed, unsigned int uncompressedSize, PSAVES saves, unsigned int numSaves);

