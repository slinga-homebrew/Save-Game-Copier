#include "backup.h"
#include "backup-saturn.h"
#include "backup-satiator.h"
#include "backup-cd.h"

// queries the saves on the backup device and fills out the saves array
int listSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    int result = 0;

    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnListSaveFiles(backupDevice, saves, numSaves);

        case SatiatorBackup:
            satiatorEnter();
            result = satiatorListSaveFiles(backupDevice, saves, numSaves);
            satiatorExit();
            return result;

        case CdMemoryBackup:
            return cdListSaveFiles(backupDevice, saves, numSaves);

        default:
            jo_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return -1;
}

// reads the specified save game from the backup device
int readSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    int result = 0;

    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnReadSaveFile(backupDevice, filename, outBuffer, outSize);

        case SatiatorBackup:
            satiatorEnter();
            result = satiatorReadSaveFile(backupDevice, filename, outBuffer, outSize);
            satiatorExit();
            return result;

        case CdMemoryBackup:
            return cdReadSaveFile(backupDevice, filename, outBuffer, outSize);

        default:
            jo_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return -1;
}

// write the save game to the backup device
int writeSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize)
{
    int result = 0;

    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnWriteSaveFile(backupDevice, filename, inBuffer, inSize);

        case SatiatorBackup:
            satiatorEnter();
            result = satiatorWriteSaveFile(backupDevice, filename, inBuffer, inSize);
            satiatorExit();
            return result;

        case CdMemoryBackup:
            return -1;

        default:
            jo_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }
}

// delete the save from the backup device
int deleteSaveFile(int backupDevice, char* filename)
{
    int result = 0;

    switch(backupDevice)
    {
        case JoInternalMemoryBackup:
        case JoCartridgeMemoryBackup:
        case JoExternalDeviceBackup:
            return saturnDeleteSaveFile(backupDevice, filename);

        case SatiatorBackup:
            satiatorEnter();
            result = satiatorDeleteSaveFile(backupDevice, filename);
            satiatorExit();
            return result;

        case CdMemoryBackup:
            return -1;

        default:
            jo_core_error("Invalid backup device specified!! %d\n", backupDevice);
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
            jo_core_error("Invalid device to format!!");
            return -1;
        }
    }

    result = jo_backup_mount(backupDevice);
    if(result == false)
    {
        char* deviceName = NULL;
        getBackupDeviceName(backupDevice, &deviceName);

        jo_core_error("Failed to mount %s!!", deviceName);
        return -2;
    }

    result = jo_backup_format_device(backupDevice);
    if(result == false)
    {
        jo_core_error("Failed to format device!!");
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
            *deviceName = "Internal Memory";
            break;
        case JoCartridgeMemoryBackup:
            *deviceName = "Cartridge Memory";
            break;
        case JoExternalDeviceBackup:
            *deviceName = "External Device";
            break;
        case SatiatorBackup:
            *deviceName = "Satiator";
            break;
        case CdMemoryBackup:
            *deviceName = "CD Memory";
            break;

        default:
            jo_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return 0;
}

