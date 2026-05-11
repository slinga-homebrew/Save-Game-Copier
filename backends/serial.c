#include <jo/serial.h>
#include "serial.h"

#define SERIAL_SEND_BUSY        (-2)
#define MAX_SEND_BUSY_ERRORS    (0x400)

bool g_serial_initialized = false;

static void init_serial(void);

// Check if the serial interface is available
bool serialIsBackupDeviceAvailable(int backupDevice)
{
    if(backupDevice != SerialBackup)
    {
        return false;
    }

    // no real way to check without something responding on the other side
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

    init_serial();

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

    init_serial();

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
    unsigned int bytesWritten = 0; 
    int consecutiveErrors = 0;
    int result = 0;

    if(backupDevice != SerialBackup)
    {
        return -1;
    }

    init_serial();

    if(filename == NULL)
    {
        sgc_core_error("serialWriteSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(saveData == NULL || saveDataLen == 0)
    {
        sgc_core_error("serialWriteSaveFile: Save file size is invalid %d!!", saveDataLen);
        return -2;
    }
    
    while(bytesWritten < saveDataLen)
    {
        result = jo_serial_send_byte(saveData[bytesWritten]);
        if(result != 0)
        {
            // retry if the serial link is busy
            if(result == SERIAL_SEND_BUSY)
            {
                consecutiveErrors++;

                // check if we had too many errors in a row
                if(consecutiveErrors >= MAX_SEND_BUSY_ERRORS)
                {
                    sgc_core_error("Serial busy %d %d", result, bytesWritten);
                    return result;
                }

                continue;
            }
            else
            {                
                sgc_core_error("Serial error %d %d", result, bytesWritten);
                return result;
            }            
        }

        // reset error count after every successful send
        consecutiveErrors = 0;
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

    init_serial();

    if(filename == NULL)
    {
        sgc_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    // not implemented yet
    return -1;
}

// initialize the serial link before using
static void init_serial(void)
{
    if(g_serial_initialized == true)
    {
        return;
    }

    jo_serial_async_init();

    g_serial_initialized = true;
}
