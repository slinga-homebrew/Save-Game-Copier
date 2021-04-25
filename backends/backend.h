#pragma once

#include <jo/jo.h>
#include <STRING.H>
#include "../util.h"
#include "../bup_header.h"

#define MAX_SAVE_SIZE           (256 * 1024) // according to Cafe-Alpha this is the maximum size supported by the BIOS
#define MAX_SAVE_FILENAME       12
#define MAX_SAVE_COMMENT        11
#define MAX_FILENAME            32
#define MAX_SAVES               255

// all devices should standardize on this directory
// for storing saves
#define SAVES_DIRECTORY "SATSAVES"

#define SatiatorBackup (JoExternalDeviceBackup + 1)
#define CdMemoryBackup (SatiatorBackup + 1)
#define MemoryBackup (CdMemoryBackup + 1)
#define MODEBackup (MemoryBackup + 1)
#define ActionReplayBackup (MODEBackup + 1)

// meta data related to save files
typedef struct  _SAVES {
    char filename[MAX_FILENAME]; // filename on the medium. Will have .BUP extension on CD FS and ODEs.
    char name[MAX_SAVE_FILENAME]; // selected save name
    char comment[MAX_SAVE_COMMENT]; // selected save comment
    unsigned char language;
    unsigned int date;
    unsigned int datasize;
    unsigned short blocksize;
} SAVES, *PSAVES;

typedef int (*BACKUP_LIST_FN)(int backupDevice, PSAVES saves, unsigned int numSaves);
typedef int (*BACKUP_READ_FN)(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize);
typedef int (*BACKUP_WRITE_FN)(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize);
typedef int (*BACKUP_DELETE_FN)(int backupDevice, char* filename);
typedef int (*BACKUP_FORMAT_FN)(int backupDevice);

typedef struct _BACKUP_MEDIUM
{
    int backupDevice;
    char* deviceName;

    BACKUP_LIST_FN listSaveFiles;
    BACKUP_READ_FN readSaveFile;
    BACKUP_WRITE_FN writeSaveFile;
    BACKUP_DELETE_FN deleteSaveFile;
    BACKUP_FORMAT_FN formatDevice;
} BACKUP_MEDIUM, *PBACKUP_MEDIUM;

// access the save data
bool isBackupDeviceAvailable(int backupDevice);
int listSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves);
int readSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize);
int writeSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize);
int deleteSaveFile(int backupDevice, char* filename);
int formatDevice(int backupDevice);

// helper functions
int getBackupDeviceName(unsigned int backupDevice, char** deviceName);
bool isFileBUPExt(char* filename);
int parseBupHeaderValues(PBUP_HEADER bupHeader, unsigned int totalBupSize, char* saveName, char* saveComment, unsigned char* saveLanguage, unsigned int* saveDate, unsigned int* saveSize, unsigned short* saveBlocks);

// prototypes to keep compiler happy
int snprintf(char *str, size_t size, const char *format, ...);

