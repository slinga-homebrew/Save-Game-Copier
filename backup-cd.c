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

// queries the saves on the CD device and fills out the fileSaves array
int cdListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    int count = 0;
    GfsHn gfs = 0;
    char* sub_dir = "SAVES";

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

        filename = (char*)GFS_IdToName(i+2);

        if(filename && numBytes)
        {
            char tempName[MAX_SAVE_FILENAME] = {0};

            cdToBackupName(filename, tempName);

            strncpy((char*)&saves[count].filename, tempName, MAX_SAVE_FILENAME);
            saves[i].datasize = numBytes;
            saves[i].blocksize = 0; // blocksize on the cd doesn't matter
            count++;
        }

        GFS_Close(gfs);
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
    char cdFilename[MAX_SAVE_FILENAME + 1] = {0};
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

    backupNameToCDName(filename, cdFilename);

    if(sub_dir != JO_NULL)
    {
        jo_fs_cd(sub_dir);
    }

    saveData = (unsigned char*)jo_fs_read_file(cdFilename, &length);

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

// removes the "." in an 8.3 filename
void cdToBackupName(char* input, char* output)
{
    int j = 0;

    for(int i = 0; i < MAX_SAVE_FILENAME; i++)
    {
        if(input[i] == '.')
        {
            continue;
        }

        if(input[i] == '\0')
        {
            break;
        }

        // not a period or a NULL, copy it over
        output[j] = input[i];
        j++;
    }

    return;
}

// converts an 11 character filename to 8.3
// output buffer must be at least MAX_SAVE_FILENAME + 1
void backupNameToCDName(char* input, char* output)
{
    for(int i = 0; i < MAX_SAVE_FILENAME; i++)
    {
        if(i < 8)
        {
            output[i] = input[i];
        }
        else
        {
            output[i + 1] = input[i];
        }
    }

    output[8] = '.';
    return;
}
