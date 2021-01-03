#include "backup-cd.h"

// always return true for saves being present
bool cdIsBackupDeviceAvailable(int backupDevice)
{
    if(backupDevice != CdMemoryBackup)
    {
        return false;
    }

    return true;
}

// queries the saves on the CD device and fills out the saves array
int cdListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    int count = 0;
    GfsHn gfs = 0;
    char* sub_dir = "SAVES";
    BUP_HEADER bupHeader = {0};

    if(backupDevice != CdMemoryBackup)
    {
        return -1;
    }

    if (sub_dir != JO_NULL)
    {
        jo_fs_cd(sub_dir);
    }

    // loop through the files on the directory
    for(unsigned int i = 0; i < numSaves; i++)
    {
        int numBytes = 0;
        char* filename = NULL;

        gfs = GFS_Open(i+2);
        if(gfs == NULL)
        {
            break;
        }

        // query the file size
        GFS_GetFileInfo(gfs, NULL, NULL, (Sint32*)&numBytes, NULL);
        GFS_Close(gfs);

        filename = (char*)GFS_IdToName(i+2);

        if(filename && numBytes)
        {
            bool result = false;

            result = isFileBUPExt(filename);
            if(result == false)
            {
                // not a .BUP file, skip
                //sgc_core_error("Not a .BUP %s", filename);
                continue;
            }

            result = cdReadBUPHeader(filename, &bupHeader);
            if(result != 0)
            {
                sgc_core_error("Failed to read .BUP %s (%d)", filename, result);
                continue;
            }

            result = parseBupHeaderValues(&bupHeader, numBytes, saves[count].name, saves[count].comment, &saves[count].language, &saves[count].date, &saves[count].datasize, &saves[count].blocksize);
            if(result != 0)
            {
                sgc_core_error("parseBup fail %s (%d)", filename, result);
                continue;
            }

            // copy over the filename as well
            strncpy((char*)saves[count].filename, filename, MAX_FILENAME);
            count++;
        }
    }

    if (sub_dir != JO_NULL)
    {
        jo_fs_cd(JO_PARENT_DIR);
    }

    return count;
}

// read the save game
int cdReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    unsigned char* saveData = NULL;
    char* sub_dir = "SAVES";
    int length = 0;

    if(backupDevice != CdMemoryBackup)
    {
        return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    if(outSize == 0 || outSize > MAX_SAVE_SIZE)
    {
        sgc_core_error("Save file size is invalid %d!!", outSize);
        return -2;
    }

    if(sub_dir != JO_NULL)
    {
        jo_fs_cd(sub_dir);
    }

    saveData = (unsigned char*)jo_fs_read_file(filename, &length);

    if(sub_dir != JO_NULL)
    {
        jo_fs_cd(JO_PARENT_DIR);
    }

    if(saveData != NULL)
    {
        // copy the save game data and free the jo engine buffer
        memcpy(outBuffer, saveData, length);
        jo_free(saveData);
        return 0;
    }

    // failed to read the save file
    return -3;
}

// read the bup header
int cdReadBUPHeader(char* filename, PBUP_HEADER bupHeader)
{
    int result = 0;
    jo_file joFile = {0};
    bool openedFile = false;

    if(!filename || !bupHeader)
    {
        return -1;
    }

    result = jo_fs_open(&joFile, filename);
    if(result != true)
    {
        sgc_core_error("failed to open %s", filename);
        result = -2;
        goto exit;
    }

    openedFile = true;

    // read the .BUP header
    result = jo_fs_read_next_bytes(&joFile, (char*)bupHeader, sizeof(BUP_HEADER));
    if(result < 0)
    {
        sgc_core_error("failed to read bup header %d", result);
        result = -3;
        goto exit;
    }

    if(result < (int)sizeof(BUP_HEADER))
    {
        sgc_core_error("bup header is too small");
        result = -4;
        goto exit;
    }

    result = 0;

exit:
    if(openedFile == true)
    {
        jo_fs_close(&joFile);
    }

    return result;
}

