#include <jo/modem.h>
#include "modem.h"

#define CONNECT_DIAL_NUMBER   "199407"
#define CONNECT_DIAL_TIMEOUT  180000000  /* ~60 seconds at 28.6MHz */
#define MAX_SEND_ERRORS       0x400

saturn_uart16550_t g_uart = {0};

static bool g_modem_initialized = false;

static int connect_modem(void);

// Check if the modem interface is available
bool modemIsBackupDeviceAvailable(int backupDevice)
{
    if(backupDevice != ModemBackup)
    {
        return false;
    }

    return modem_is_present();
}

// List the remote saves
// TODO: this will involve having something on the other end respond
int modemListSaveFiles(int backupDevice, PSAVES fileSaves, unsigned int numSaves)
{
    UNUSED_ARG(fileSaves);
    UNUSED_ARG(numSaves);

    unsigned int count = 0;

    if(backupDevice != ModemBackup)
    {
        return -1;
    }

    //connect_modem();

    count = 0;
    
    return count;
}

// Read the save
// TODO: this will involve having something on the other end respond
int modemReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    UNUSED_ARG(filename);
    UNUSED_ARG(outSize);

    if(backupDevice != ModemBackup)
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
int modemWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen)
{   
    unsigned int bytesWritten = 0; 
    int consecutiveErrors = 0;
    int result = 0;

    if(backupDevice != ModemBackup)
    {
        return -1;
    }
    
    result = connect_modem();
    if(result != 0)
    {
        sgc_core_error("connect: %d", result);
        return result;
    }

    if(filename == NULL)
    {
        sgc_core_error("modemWriteSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(saveData == NULL || saveDataLen == 0)
    {
        sgc_core_error("modemWriteSaveFile: Save file size is invalid %d!!", saveDataLen);
        return -2;
    }
    
    while(bytesWritten < saveDataLen)
    {        
        result = modem_send_bytes(&g_uart, &saveData[bytesWritten], sizeof(unsigned char));
        if(result != 1)
        {
            consecutiveErrors++;
            // check if we had too many errors in a row
            if(consecutiveErrors >= MAX_SEND_ERRORS)
            {
                sgc_core_error("modem send %d %d", result, bytesWritten);
                return -3;
            }
            else
            {
                continue;
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
int modemDeleteSaveFile(int backupDevice, char* filename)
{
    if(backupDevice != ModemBackup)
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

// initialize the modem link before using
static int connect_modem(void)
{
    bool bResult = false;
    int result = 0;

    if(g_modem_initialized == true)
    {
        return 0;
    }

    bResult = modem_get_uart(&g_uart);
    if(bResult == false)
    {
        return -1;
    }

    for(unsigned int i = 0; i < MAX_SEND_ERRORS; i++)
    {
        slSynch();
        result = modem_probe(&g_uart);
        if(result == MODEM_OK)
        {
            break;
        }
    }

    if(result != MODEM_OK)
    {
        return -2;
    }

    slSynch();
    result = modem_init(&g_uart);
    if(result != MODEM_OK)
    {
        return -3;
    }

    slSynch();
    result = modem_dial(&g_uart, CONNECT_DIAL_NUMBER, CONNECT_DIAL_TIMEOUT);
    if(result != MODEM_CONNECT)
    {
        sgc_core_error("dial error: %d", result);
        return -4;
    }    

    modem_flush_input(&g_uart);
    
    g_modem_initialized = true;

    return 0;
}
