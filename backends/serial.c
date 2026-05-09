#include <jo/serial.h>
#include "serial.h"

// Check if the serial interface is available
// TODO: this will involve having something on the other end respond
bool serialIsBackupDeviceAvailable(int backupDevice)
{
    if(backupDevice != SerialBackup)
    {
        return false;
    }

    // TODO: not here
    // initialize serial
	jo_serial_async_init();

    return true;
}

// List the remote saves
// TODO: this will involve having something on the other end respond
int serialListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves)
{
    UNUSED_ARG(fileSaves);
    UNUSED_ARG(numSaves);

    unsigned int count = 0;

    if(backupDevice != SerialBackup)
    {
        return -1;
    }

    count = 0;
    
    return count;
}

// Read the save
// TODO: this will involve having something on the other end respond
int serialReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    UNUSED_ARG(filename);
    UNUSED_ARG(outSize);

    if(backupDevice != SerialBackup)
    {
        return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    // not implemented yet
    return -1;
}

// Write the save
int serialWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen)
{
    int result = 0;

    if(backupDevice != SerialBackup)
    {
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("writeSatiatorSaveData: Save file data buffer is NULL!!");
        return -1;
    }

    if(saveData == NULL || saveDataLen == 0)
    {
        sgc_core_error("writeSatiatorSaveData: Save file size is invalid %d!!", saveDataLen);
        return -2;
    }

    for(unsigned int bytesWritten = 0; bytesWritten < saveDataLen; )
    {
        result = jo_serial_send_byte(saveData[bytesWritten]);
        if(result != 0)
        {
            return -3;
        }

        bytesWritten += 1;
    }

    return 0;
}

// Delete the save
// TODO: this will involve having something on the other end respond
int serialDeleteSaveFile(int backupDevice, char* filename)
{
    if(backupDevice != SerialBackup)
    {
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    // not implemented yet
    return -1;
}
