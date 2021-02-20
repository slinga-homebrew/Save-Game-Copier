#include "backup.h"
#include "backup-saturn.h"
#include "backup-mode.h"
#include "backup-satiator.h"
#include "backup-cd.h"

// returns true if the backup device is found
bool isBackupDeviceAvailable(int backupDevice)
{
    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnIsBackupDeviceAvailable(backupDevice);

        case SatiatorBackup:
        {
            bool result;

            satiatorEnter();
            result = satiatorIsBackupDeviceAvailable(backupDevice);
            satiatorExit();

            return result;
        }

        case MODEBackup:
            return modeIsBackupDeviceAvailable(backupDevice);

        case CdMemoryBackup:
            return true; // always assume CD backups are available

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return -1;
}

// queries the saves on the backup device and fills out the saves array
int listSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnListSaveFiles(backupDevice, saves, numSaves);

        case SatiatorBackup:
        {
            int result;

            satiatorEnter();
            result = satiatorListSaveFiles(backupDevice, saves, numSaves);
            satiatorExit();

            return result;
        }

        case MODEBackup:
            return modeListSaveFiles(backupDevice, saves, numSaves);

        case CdMemoryBackup:
            return cdListSaveFiles(backupDevice, saves, numSaves);

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return -1;
}

// reads the specified save game from the backup device
int readSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnReadSaveFile(backupDevice, filename, outBuffer, outSize);

        case SatiatorBackup:
        {
            int result;

            satiatorEnter();
            result = satiatorReadSaveFile(backupDevice, filename, outBuffer, outSize);
            satiatorExit();

            return result;
        }

        case MODEBackup:
            return modeReadSaveFile(backupDevice, filename, outBuffer, outSize);

        case CdMemoryBackup:
            return cdReadSaveFile(backupDevice, filename, outBuffer, outSize);

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return -1;
}

// write the save game to the backup device
int writeSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize)
{
    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnWriteSaveFile(backupDevice, filename, inBuffer, inSize);

        case SatiatorBackup:
        {
            int result;

            satiatorEnter();
            result = satiatorWriteSaveFile(backupDevice, filename, inBuffer, inSize);
            satiatorExit();

            return result;
        }

        case MODEBackup:
            return modeWriteSaveFile(backupDevice, filename, inBuffer, inSize);

        case CdMemoryBackup:
            return -1;

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }
}

// delete the save from the backup device
int deleteSaveFile(int backupDevice, char* filename)
{
    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnDeleteSaveFile(backupDevice, filename);

        case SatiatorBackup:
        {
            int result;

            satiatorEnter();
            result = satiatorDeleteSaveFile(backupDevice, filename);
            satiatorExit();

            return result;
        }

        case MODEBackup:
            return modeDeleteSaveFile(backupDevice, filename);

        case CdMemoryBackup:
            return -1;

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return -1;
}

// format a backup device
int formatDevice(int backupDevice)
{
    bool result = false;

    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            break;
        default:
        {
            sgc_core_error("Invalid device to format!!");
            return -1;
        }
    }

    result = jo_backup_mount(backupDevice);
    if(result == false)
    {
        char* deviceName = NULL;
        getBackupDeviceName(backupDevice, &deviceName);

        sgc_core_error("Failed to mount %s!!", deviceName);
        return -2;
    }

    result = jo_backup_format_device(backupDevice);
    if(result == false)
    {
        sgc_core_error("Failed to format device!!");
        return -3;
    }

    return 0;
}

// get device name from device id
int getBackupDeviceName(unsigned int backupDevice, char** deviceName)
{
    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
            *deviceName = "Internal Backup Memory";
            break;
        case JoCartridgeMemoryBackup:
            *deviceName = "Cartridge Backup Memory";
            break;
        case JoExternalDeviceBackup:
            *deviceName = "External Backup Device";
            break;
        case SatiatorBackup:
            *deviceName = "Satiator";
            break;
        case MODEBackup:
            *deviceName = "MODE";
            break;
        case CdMemoryBackup:
            *deviceName = "CD File System";
            break;
        case MemoryBackup:
            *deviceName = "(Advanced) RAM";
            break;

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return 0;
}

// check if the filename ends with ".BUP"
bool isFileBUPExt(char* filename)
{
    unsigned int len = 0;
    char* ext = NULL;
    int result = 0;

    if(!filename)
    {
        sgc_core_error("Filename cannot be NULL");
        return false;
    }

    len = strlen(filename);
    if(len < sizeof(BUP_EXTENSION))
    {
        return false;
    }

    ext = &filename[len - strlen(BUP_EXTENSION)];
    result = strcmp(ext, BUP_EXTENSION);
    if(result == 0)
    {
        return true;
    }

    return false;
}

// validates the BUP header and extracts the various fields contained within
int parseBupHeaderValues(PBUP_HEADER bupHeader, unsigned int totalBupSize, char* saveName, char* saveComment, unsigned char* saveLanguage, unsigned int* saveDate, unsigned int* saveSize, unsigned short* saveBlocks)
{
    int result = 0;

    if(!bupHeader || !totalBupSize || !saveName || !saveComment || !saveLanguage || !saveDate || !saveSize || !saveBlocks)
    {
        return -1;
    }

    if(totalBupSize < BUP_HEADER_SIZE)
    {
        return -2;
    }

    result = memcmp(bupHeader->magic, VMEM_MAGIC_STRING, sizeof(bupHeader->magic));
    if(result != 0)
    {
        return -3;
    }

    if(totalBupSize != bupHeader->dir.datasize + BUP_HEADER_SIZE)
    {
       // return -4
    }

    memcpy(saveName, bupHeader->dir.filename, sizeof(bupHeader->dir.filename));
    saveName[sizeof(bupHeader->dir.filename) -1] = '\0';

    memcpy(saveComment, bupHeader->dir.comment, sizeof(bupHeader->dir.comment));
    saveComment[sizeof(bupHeader->dir.comment) -1] = '\0';

    *saveLanguage = bupHeader->dir.language;
    *saveDate = bupHeader->dir.date;
    *saveSize = bupHeader->dir.datasize;
    *saveBlocks = bupHeader->dir.blocksize;

    return 0;
}
