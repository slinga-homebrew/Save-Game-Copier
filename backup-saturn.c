#include "backup-saturn.h"

# define JO_BACKUP_DRIVER_ADDR                  (*(volatile unsigned int *)(0x6000354))
/** @brief Helper to get backup driver functions addresses */
# define JO_BACKUP_FUNCTION_ADDR(INDEX)         (*(unsigned int *)(JO_BACKUP_DRIVER_ADDR + INDEX))

# define BUP_SelPart(DEVICE, PARTITION)	\
((int (*)(unsigned int, unsigned int))JO_BACKUP_FUNCTION_ADDR(4))(DEVICE, PARTITION)

// queries the saves on the backup device and fills out the fileSaves array
int saturnListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    jo_list saveFilenames = {0};
    bool result = false;
    int count = 0;

    jo_list_init(&saveFilenames);

    // mount the backup device
    result = jo_backup_mount(backupDevice);
    if(result == false)
    {
        // Don't need this error message because jo engine already errors
        //jo_core_error("Failed to mount backup device %d!!", backupDevice);
        return -1;
    }

    if(backupDevice == JoExternalDeviceBackup)
    {
        int part = BUP_SelPart(2, 1);
        if(part != 0)
        {
            jo_core_error("BUP_SelPart returned 0x%x", part);
            return -1;
        }
        else
        {
            jo_core_error("BUP_SelPart returned success");
        }
    }

    // get a list of files from the backup device
    result = jo_backup_read_device(backupDevice, &saveFilenames);
    if(result == false)
    {
        jo_list_free_and_clear(&saveFilenames);
        return -1;
    }

    for(unsigned int i = 0; i < (unsigned int)saveFilenames.count && i < numSaves; i++)
    {
        unsigned int numBytes = 0;
        unsigned int numBlocks = 0;

        char* filename = jo_list_at(&saveFilenames, i)->data.ch_arr;

        if(filename == NULL)
        {
            jo_core_error("readSaveFiles list is corrupt!!");
            return -1;
        }

        result = jo_backup_get_file_size(backupDevice, filename, &numBytes, &numBlocks);
        if(result == false)
        {
            jo_core_error("Failed to read file size!!");
            return -1;
        }

        strncpy((char*)&saves[i].filename, filename, MAX_SAVE_FILENAME);
        saves[i].datasize = numBytes;
        saves[i].blocksize = numBlocks;
        count++;
    }

    jo_list_free_and_clear(&saveFilenames);

    return count;
}


// copies the specified save gane to the saveFileData buffer
int saturnReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outBufSize)
{
    unsigned char* saveData = NULL;

    if(outBuffer == NULL || filename == NULL)
    {
        jo_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    if(outBufSize == 0 || outBufSize > MAX_SAVE_SIZE)
    {
        jo_core_error("Save file size is invalid %d!!", outBufSize);
        return -2;
    }

    if(backupDevice == JoExternalDeviceBackup)
    {
        int part = BUP_SelPart(2, 1);
        if(part != 0)
        {
            jo_core_error("BUP_SelPart returned 0x%x", part);
            return -1;
        }
        else
        {
            jo_core_error("BUP_SelPart returned success");
        }
    }

    // read the file from the backup device
    // jo engine mallocs a buffer for us
    saveData = jo_backup_load_file_contents(backupDevice, filename, &outBufSize);
    if(saveData == NULL)
    {
        jo_core_error("Failed to read save file!!");
        return -3;
    }

    // copy the save game data and free the jo engine buffer
    memcpy(outBuffer, saveData, outBufSize);
    jo_free(saveData);

    return 0;
}

int saturnWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen)
{
    bool result;

    // mount the backup device
    result = jo_backup_mount(backupDevice);
    if(result == false)
    {
        jo_core_error("Failed to mount backup device %d!!", backupDevice);
        return -1;
    }

    if(backupDevice == JoExternalDeviceBackup)
    {
        int part = BUP_SelPart(2, 1);
        if(part != 0)
        {
            jo_core_error("BUP_SelPart returned 0x%x", part);
            return -1;
        }
        else
        {
            jo_core_error("BUP_SelPart returned success");
        }
    }

    result = jo_backup_save_file_contents(backupDevice, filename, "SGC", saveData, saveDataLen);
    if(result == false)
    {
        jo_core_error("Failed to write save backup device %d!!", backupDevice);
        return -1;
    }

    return 0;
}

int saturnDeleteSaveFile(int backupDevice, char* filename)
{
    bool result;

    // mount the backup device
    result = jo_backup_mount(backupDevice);
    if(result == false)
    {
        jo_core_error("Failed to mount backup device %d!!", backupDevice);
        return -1;
    }

    if(backupDevice == JoExternalDeviceBackup)
    {
        int part = BUP_SelPart(2, 1);
        if(part != 0)
        {
            jo_core_error("BUP_SelPart returned 0x%x", part);
            return -1;
        }
        else
        {
            jo_core_error("BUP_SelPart returned success");
        }
    }

    result = jo_backup_delete_file(backupDevice, filename);
    if(result == false)
    {
        return -1;
    }

    return 0;
}
