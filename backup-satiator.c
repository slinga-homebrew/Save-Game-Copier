#include "backup-satiator.h"
#include "satiator/satiator.h"

// queries the saves on the Satiator device and fills out the saves array
// BUGBUG: code copied from satiator-menu/main.c and needs ot be under the MPL
int satiatorListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    int result = 0;
    unsigned int count = 0;
    char statbuf[280] = {0};
    s_stat_t *st = (s_stat_t*)statbuf;
    int len = 0;

    if(backupDevice != SatiatorBackup)
    {
        return -1;
    }

    result = s_opendir(".");
    if(result != 0)
    {
        jo_core_error("readSatiatorSaveFiles: Failed to open SAVES directory");
        //return -2;
    }

    // loop through the files in the directory
    while ((len = s_stat(NULL, st, sizeof(statbuf)-1)) > 0)
    {
        st->name[len] = 0;

        //skip directories
        if(st->attrib & AM_DIR)
        {
            continue;
        }

        // skip . and .. (and anything beginning with .)
        if (st->name[0] == '.')
        {
            continue;
        }

        strncpy((char*)&saves[count].filename, st->name, MAX_SAVE_FILENAME);
        saves[count].datasize = st->size;
        saves[count].blocksize = 0; // blocksize on the Satiator doesn't matter
        count++;

        if(count >= numSaves)
        {
            break;
        }
    }

    // BUGBUG: close the directory??
    return count;
}

// copies the specified Satiator save game to the saveFileData buffer
int satiatorReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    int result = 0;
    int fd = 0;

    if(backupDevice != SatiatorBackup)
    {
        return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        jo_core_error("readSatiatorSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(outSize == 0 || outSize > MAX_SAVE_SIZE)
    {
        jo_core_error("readSatiatorSaveFile: Save file size is invalid %d!!", outSize);
        return -2;
    }

    fd = s_open(filename, FA_READ);
    if(fd < 0)
    {
        jo_core_error("readSatiatorSaveFile: Failed to open satiator file!!");
        return -2;
    }

    for(unsigned int bytesRead = 0; bytesRead < outSize; )
    {
        unsigned int count;

        count = MIN(outSize - bytesRead, 2048);

        // BUGBUG: fix this
        jo_core_error("sat read: %x %x %x", bytesRead, count, outSize);
        result = s_read(fd, outBuffer + bytesRead, count);
        jo_core_error("sat read res: %x %x %x", result, bytesRead, count);

        if(result <= 0)
        {
            jo_core_error("Bad read result: %x", result);
            result = -1;
            break;
        }

        bytesRead += count;
    }

    s_close(fd);

    if(result < 0)
    {
        jo_core_error("readSatiatorSaveFile: Failed to read satiator file!!");
        return -3;
    }

    return 0;
}

// read the save game
int satiatorWriteSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize)
{
    int result = 0;
    int fd = 0;

    if(backupDevice != SatiatorBackup)
    {
        return -1;
    }

    if(inBuffer == NULL || filename == NULL)
    {
        jo_core_error("writeSatiatorSaveData: Save file data buffer is NULL!!");
        return -1;
    }

    if(inSize == 0 || inSize > MAX_SAVE_SIZE)
    {
        jo_core_error("writeSatiatorSaveData: Save file size is invalid %d!!", inSize);
        return -2;
    }

    fd = s_open(filename, FA_WRITE|FA_CREATE_ALWAYS);
    if(fd < 0)
    {
        jo_core_error("writeSatiatorSaveData: Failed to open satiator file!!");
        return -2;
    }

    for(unsigned int bytesWritten = 0; bytesWritten < inSize; )
    {
        unsigned int count;

        count = MIN(inSize - bytesWritten, 2048);

        // BUGBUG: fix this
        jo_core_error("sat write: %x %x", bytesWritten, count);
        result = s_write(fd, inBuffer + bytesWritten, count);
        jo_core_error("sat write res: %x %x %x", result, bytesWritten, count);

        s_sync(fd);

        if(result <= 0)
        {
            jo_core_error("Bad write result: %x", result);
            result = -1;
            break;
        }

        bytesWritten += count;
    }

    s_close(fd);

    if(result < 0)
    {
        jo_core_error("writeSatiatorSaveData: Failed to read satiator file!!");
        return -3;
    }

    return 0;
}

// delete the save
int satiatorDeleteSaveFile(int backupDevice, char* filename)
{
    int result = 0;

    if(backupDevice != SatiatorBackup)
    {
        return -1;
    }

    if(filename == NULL)
    {
        jo_core_error("deleteSatiatorSaveData: Filename is NULL!!");
        return -1;
    }

    result = s_unlink(filename);

    if(result < 0)
    {
        jo_core_error("deleteSatiatorSaveData: Failed to read satiator file!!");
        return -3;
    }

    return 0;
}

// enable satiator extra mode
// without this you cannot access the filesystem
int satiatorEnter()
{
    int result;

    result = s_mode(S_MODE_USBFS);

    result = s_chdir("/SAVES");
    //jo_core_error("enter chdir: %x", result);

    return 0;
}

// exit satiator extra mode
// BUGBUG: this does not appear to work and ends up with the ISO no longer being accessible
int satiatorExit()
{
    int result;

    result = s_chdir("..");
    //jo_core_error("exit chdir: %x", result);

    result = s_mode(S_MODE_CDROM);
    //jo_core_error("cdrom: %x", result);
    return 0;
}
