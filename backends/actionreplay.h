#pragma once

#include "backend.h"

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
#define RLE01_MAX_COUNT                 0x100
#define RLE_MAX_REPEAT                  0xFF

#pragma pack(1)
typedef struct _RLE01_HEADER
{
    char compressionMagic[5]; // should be "RLE01"
    unsigned char rleKey; // key used to compress the datasize
    unsigned int compressedSize; // size of the compressed data
}RLE01_HEADER, *PRLE01_HEADER;
#pragma pack()

bool actionReplayIsBackupDeviceAvailable(int backupDevice);
int actionReplayListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves);
int actionReplayReadSaveFile(int backupDevice, char* filename, unsigned char* ouBuffer, unsigned int outBufSize);
int actionReplayWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen);
int actionReplayDeleteSaveFile(int backupDevice, char* filename);

// utility functions
int decompressPartition(unsigned char *src, unsigned int srcSize, unsigned char **dest, unsigned int* destSize);
int decompressRLE01(unsigned char rleKey, unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int* bytesNeeded);
int calcRLEKey(unsigned char* src, unsigned int size, unsigned char* key);
int compressRLE01(unsigned char rleKey, unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int* bytesNeeded);

