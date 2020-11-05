#include "backup.h"
#include "backup-saturn.h"
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
            return satiatorIsBackupDeviceAvailable(backupDevice);

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
            return satiatorListSaveFiles(backupDevice, saves, numSaves);

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
            return satiatorReadSaveFile(backupDevice, filename, outBuffer, outSize);

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
            return satiatorWriteSaveFile(backupDevice, filename, inBuffer, inSize);

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
            return satiatorDeleteSaveFile(backupDevice, filename);

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
            *deviceName = "CD File System";
            break;
        case MemoryBackup:
            *deviceName = "RAM";
            break;

        default:
            sgc_core_error("Invalid backup device specified!! %d\n", backupDevice);
            return -1;
    }

    return 0;
}

