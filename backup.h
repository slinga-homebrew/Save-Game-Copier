#pragma once

#include <jo/jo.h>

#define MAX_SAVE_SIZE           (512 * 1024)/4 // according to Cafe-Alpha this is the maximum size supported by the bios
#define MAX_SAVE_FILENAME       12
#define MAX_SAVES               255

#define SatiatorBackup (JoExternalDeviceBackup + 1)
#define CdMemoryBackup (SatiatorBackup + 1)

// meta data related to save files
typedef  struct  _SAVES {
    char filename[MAX_SAVE_FILENAME];
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

//BACKUP_MEDIUM g_backupSaturn = {

// access the save data
int listSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves);
int readSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize);
int writeSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize);
int deleteSaveFile(int backupDevice, char* filename);
int formatDevice(int backupDevice);

// helper functions
int getBackupDeviceName(unsigned int backupDevice, char** deviceName);

// prototypes to keep compiler happy
void *memcpy(void *dest, const void *src, unsigned int n);
char *strncpy(char *dest, const char *src, unsigned int n);
