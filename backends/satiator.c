#include "satiator.h"
#include "satiator/satiator.h"

// returns true if the backup device is found
bool satiatorIsBackupDeviceAvailable(int backupDevice)
{
    int result;

    if(backupDevice != SatiatorBackup)
    {
        return false;
    }

    result = satiatorEnter();
    if(result != 0)
    {
        // failed to find Satiator
        return false;
    }

    // found Satiator
    return true;
}

// queries the saves on the Satiator device and fills out the saves array
// BUGBUG: code copied from satiator-menu/main.c and needs to be under the MPL
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

    result = satiatorEnter();
    if(result != 0)
    {
        sgc_core_error("Failed to detect satiator");
        return -1;
    }

    result = s_opendir(".");
    if(result != 0)
    {
        sgc_core_error("readSatiatorSaveFiles: Failed to open SATSAVES directory");
        return -2;
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

        result = isFileBUPExt(st->name);
        if(result == false)
        {
            // not a .BUP file, skip
            continue;
        }

        strncpy((char*)saves[count].filename, st->name, MAX_FILENAME);
        saves[count].filename[MAX_FILENAME - 1] = '\0';
        saves[count].datasize = st->size - sizeof(BUP_HEADER);
        saves[count].blocksize = 0; // blocksize on the Satiator doesn't matter
        count++;

        if(count >= numSaves)
        {
            break;
        }
    }

    // for each file found, read the BUP header and parse the metadata
    for(unsigned int i = 0; i < count; i++)
    {
        BUP_HEADER bupHeader = {0};

        result = satiatorReadBUPHeader(saves[i].filename, &bupHeader);
        if(result != 0)
        {
            sgc_core_error("bup header %s", saves[i].filename);
            continue;
        }

        result = parseBupHeaderValues(&bupHeader, saves[count].datasize + sizeof(BUP_HEADER), saves[i].name, saves[i].comment, &saves[i].language, &saves[i].date, &saves[i].datasize, &saves[i].blocksize);
        if(result != 0)
        {
            sgc_core_error("Failed with %d", result);


            // BUGBUG: handle error conditions gracefully
            strncpy(saves[i].name, "Error", MAX_SAVE_FILENAME);

            continue;
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

    result = satiatorEnter();
    if(result == 0)
    {
        // why is it failing to detect now??
        //sgc_core_error("Failed to detect satiator %d", result);
        //return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("readSatiatorSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(outSize == 0 || outSize > MAX_SAVE_SIZE)
    {
        sgc_core_error("readSatiatorSaveFile: Save file size is invalid %d!!", outSize);
        return -2;
    }

    fd = s_open(filename, FA_READ);
    if(fd < 0)
    {
        sgc_core_error("readSatiatorSaveFile: Failed to open satiator file!!");
        return -2;
    }

    for(unsigned int bytesRead = 0; bytesRead < outSize; )
    {
        unsigned int count;

        count = MIN(outSize - bytesRead, S_MAXBUF);

        // BUGBUG: fix this
        result = s_read(fd, outBuffer + bytesRead, count);
        if(result <= 0)
        {
            sgc_core_error("Bad read result: %x", result);
            s_close(fd);
            return result;
        }

        bytesRead += count;
    }

    s_close(fd);

    if(result < 0)
    {
        sgc_core_error("readSatiatorSaveFile: Failed to read satiator file!!");
        return -3;
    }

    return 0;
}

// write the save game to the Satiator
int satiatorWriteSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize)
{
    int result = 0;
    int fd = 0;

    if(backupDevice != SatiatorBackup)
    {
        return -1;
    }

    result = satiatorEnter();
    if(result != 0)
    {
        sgc_core_error("Failed to detect satiator");
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("writeSatiatorSaveData: Save file data buffer is NULL!!");
        return -1;
    }

    if(inBuffer == NULL || filename == NULL)
    {
        sgc_core_error("writeSatiatorSaveData: Save file size is invalid %d!!", inSize);
        return -2;
    }

    fd = s_open(filename, FA_WRITE|FA_CREATE_ALWAYS);
    if(fd < 0)
    {
        sgc_core_error("writeSatiatorSaveData: Failed to open satiator file!!");
        return -2;
    }

    for(unsigned int bytesWritten = 0; bytesWritten < inSize; )
    {
        unsigned int count;

        count = MIN(inSize - bytesWritten, S_MAXBUF);

        // BUGBUG: fix this
        result = s_write(fd, inBuffer + bytesWritten, count);

        s_sync(fd);

        if(result <= 0)
        {
            sgc_core_error("Bad write result: %x", result);
            s_close(fd);
            return result;
            break;
        }

        bytesWritten += count;
    }

    s_close(fd);

    if(result < 0)
    {
        sgc_core_error("writeSatiatorSaveData: Failed to read satiator file!!");
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

    result = satiatorEnter();
    if(result != 0)
    {
        sgc_core_error("Failed to detect satiator");
        //return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("deleteSatiatorSaveData: Filename is NULL!!");
        return -1;
    }

    result = s_unlink(filename);

    if(result < 0)
    {
        sgc_core_error("deleteSatiatorSaveData: Failed to read satiator file!!");
        return -3;
    }

    return 0;
}

// enable satiator extra mode
// without this you cannot access the filesystem
int satiatorEnter(void)
{
    int result;

    result = s_mode(s_api);
    if(result != 0)
    {
        return -1;
    }
    s_chdir("/SATSAVES");
    return 0;
}

// exit satiator extra mode
// BUGBUG: this does not appear to work and ends up with the ISO no longer being accessible
int satiatorExit(void)
{

    // BUGBUG: this doesn't quite work correctly
    //s_chdir("..");
    //s_mode(S_MODE_CDROM);

    return 0;
}

// read the bup header
int satiatorReadBUPHeader(char* filename, PBUP_HEADER bupHeader)
{
    int result = 0;
    int fd = 0;
    bool openedFile = false;

    if(!filename || !bupHeader)
    {
        return -1;
    }

    fd = s_open(filename, FA_READ);
    if(fd < 0)
    {
        sgc_core_error("readSatiatorSaveFile: Failed to open satiator file!!");
        return -2;
    }

    // read the .BUP header
    result = s_read(fd, (unsigned char*)bupHeader, sizeof(BUP_HEADER));
    if(result <= 0)
    {
        sgc_core_error("Bad read result: %x", result);
        result = -3;
        goto exit;
    }

    openedFile = true;

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
        s_close(fd);
    }

    return result;
}

// relaunch the satiator menu
void satiatorReboot(void)
{
    s_mode(s_api);
    for (volatile int i=0; i<2000; i++)
        ;

    int (**bios_get_mpeg_rom)(uint32_t index, uint32_t size, uint32_t addr) = (void*)0x06000298;
    (*bios_get_mpeg_rom)(2, 2, 0x200000);

    ((void(*)(void))0x200000)();

    // should never get here
}

