#include "backup-saturn.h"

// queries whether the backup device is present
bool saturnIsBackupDeviceAvailable(int backupDevice)
{
    bool isPresent = false;

    isPresent = jo_backup_mount(backupDevice);
    if(isPresent == true)
    {
        jo_backup_unmount(backupDevice);
        return true;
    }

    return false;
}

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
        sgc_core_error("Failed to mount backup device %d!!", backupDevice);
        return -1;
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
        char comment[MAX_SAVE_COMMENT] = {0};
        unsigned char language = 0;
        unsigned int date = 0;
        unsigned int numBytes = 0;
        unsigned int numBlocks = 0;

        char* filename = jo_list_at(&saveFilenames, i)->data.ch_arr;

        if(filename == NULL)
        {
            sgc_core_error("readSaveFiles list is corrupt!!");
            return -1;
        }

        // query the save metadata
        result = jo_backup_get_file_info(backupDevice, filename, comment, &language, &date, &numBytes, &numBlocks);
        if(result == false)
        {
            sgc_core_error("Failed to read file size!!");
            return -1;
        }

        // fill out the SAVES structure
        snprintf(saves[i].filename, MAX_FILENAME, "%s.BUP", filename);
        strncpy(saves[i].name, filename, MAX_SAVE_FILENAME);
        strncpy(saves[i].comment, (char*)comment, sizeof(comment));
        saves[i].language = language;
        saves[i].date = date;
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
    PBUP_HEADER temp = NULL;
    int result = 0;

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    if(outBufSize < sizeof(BUP_HEADER) || outBufSize > (MAX_SAVE_SIZE + sizeof(BUP_HEADER)))
    {
        sgc_core_error("Save file size is invalid %d!!", outBufSize);
        return -2;
    }
    temp = (PBUP_HEADER)outBuffer;

    // read the file from the backup device
    // jo engine mallocs a buffer for us
    saveData = jo_backup_load_file_contents(backupDevice, filename, &outBufSize);
    if(saveData == NULL)
    {
        sgc_core_error("Failed to read save file!!");
        return -3;
    }

    // query the save metadata
    unsigned int blockSize = 0;
    result = jo_backup_get_file_info(backupDevice, filename, (char*)&temp->dir.comment, &temp->dir.language, &temp->dir.date, &temp->dir.datasize, &blockSize);

    if(result == false)
    {
        sgc_core_error("Failed to save metadata  size!!");
        return -1;
    }
    memcpy(temp->magic, VMEM_MAGIC_STRING, VMEM_MAGIC_STRING_LEN);
    strncpy((char*)temp->dir.filename, filename, MAX_SAVE_FILENAME);
    temp->date = temp->dir.date; // date is duplicated
    temp->dir.blocksize = blockSize;

    // copy the save game data and free the jo engine buffer
    memcpy(outBuffer + sizeof(BUP_HEADER), saveData, outBufSize);
    jo_free(saveData);

    return 0;
}

int saturnWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen)
{
    bool result = false;
    PBUP_HEADER temp = NULL;
    jo_backup saveMeta = {0};

    // BUP header is required
    if(saveDataLen < sizeof(BUP_HEADER))
    {
        sgc_core_error("Invalid .BUP header");
        return -1;
    }

    temp = (PBUP_HEADER)saveData;

    // mount the backup device
    result = jo_backup_mount(backupDevice);
    if(result == false)
    {
        sgc_core_error("Failed to mount backup device %d!!", backupDevice);
        return -2;
    }

    saveMeta.backup_device = backupDevice;
    saveMeta.fname = filename;
    saveMeta.comment = (char*)temp->dir.comment;
    saveMeta.contents = saveData + sizeof(BUP_HEADER);
    saveMeta.content_size = saveDataLen - sizeof(BUP_HEADER);
    saveMeta.language_num = temp->dir.language;
    saveMeta.save_timestamp = temp->dir.date;

    result = jo_backup_save(&saveMeta);
    if(result == false)
    {
        //sgc_core_error("Failed to write save backup device %d!!", backupDevice);
        return -3;
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
        sgc_core_error("Failed to mount backup device %d!!", backupDevice);
        return -1;
    }

    result = jo_backup_delete_file(backupDevice, filename);
    if(result == false)
    {
        return -1;
    }

    return 0;
}
