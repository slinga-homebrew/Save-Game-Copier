#include "mode.h"
#include "mode/mode_intf.h"
#include "STRING.H"
#include "SEGA_CDC.H"

//
// MODE support contributed by Terraonion (https://github.com/Terraonion-dev)
//

// Adjust this to be sector aligned, as MODE transfers entire sector blocks so align to sector size (+1) add an extra sector block at the end (+16)
static struct _SatDirList SatSaves[MAX_SAVES + 1 + 16] __attribute__((section(".bss")));
static unsigned char SectorBuffer[2048] __attribute__((section(".bss")));
const char* SaveDirectory = "0:/SATSAVES";
static char tmpFilename[64] __attribute__((section(".bss")));

// returns true if the backup device is found
bool modeIsBackupDeviceAvailable(int backupDevice)
{
    struct _MountStatus* ms;
    struct _VersionInfo* vi;

    if(backupDevice != MODEBackup)
    {
        return false;
    }

    Uint32* toc = (Uint32*)SectorBuffer;

    // TOC must be at least 60 minutes for the MODE interface to work
    CDC_TgetToc(toc);

    if((toc[101] & 0xFFFFFF) < 0x040000)
    {
        return false;
    }

    // Trying to open MODE interface in a disc with a <40000h toc will hang due to the seek address wait.
    MODE_Open();

    ms = MODE_GetMountStatus();	// GetMountStatus returns NULL if the MODE response is not right
    vi = MODE_GetVersionInfo(); // Actually mi and vi will share the same pointer if right, but we are only using data from vi from now on

    modeExit();

    // found MODE
    return ms != NULL && ((vi->Ver << 8) | vi->Subver) >= 0x0104; // Needs at least 1.04 version for the File IO Interface
}

// queries the saves on the MODE and fills out the saves array
int modeListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    int len = 0;
    int result = 0;
    unsigned int count = 0;
    BUP_HEADER bupHeader = {0};

    if(backupDevice != MODEBackup)
    {
        return -1;
    }

    modeEnter();

    result = MODE_ReadFileListing(SaveDirectory, SatSaves, MAX_SAVES);

    if(result < 0)
    {
        sgc_core_error("modeListSaveFiles: %d Failed to open SAVES directory", result);
        return -2;
    }

    // loop through the files in the directory
    for (int n = 0; n < result; ++n)
    {
        // skip directories
        if (SatSaves[n].Size == 0xFFFFFFFF)
        {
            continue;
        }

        // skip . and .. (and anything beginning with .) (MODE would have already filtered them)
        if (SatSaves[n].Name[0] == '.')
        {
            continue;
        }

        len = strlen(SatSaves[n].Name);

        if (len < 4)
            continue;

        //dunno what's wrong with that function
        /*
        result = isFileBUPExt(SatSaves[n].Name);
        if(result == false)
        {
            // not a .BUP file, skip
            continue;
        }
        */

        if (memcmp(SatSaves[n].Name + len - 4, BUP_EXTENSION, 4) != 0)
        {
            continue;
        }

        strncpy((char*)saves[count].filename, SatSaves[n].Name, MAX_FILENAME);

        saves[count].datasize = SatSaves[n].Size;
        saves[count].blocksize = 0;

        count++;

        if (count >= numSaves)
        {
            break;
        }
    }

    for (unsigned int n = 0; n < count; ++n)
    {
        result = modeReadBUPHeader(saves[n].filename, &bupHeader);
        if (result != 0)
        {
            sgc_core_error("bup header %s", saves[n].filename);
            continue;
        }

        result = parseBupHeaderValues(&bupHeader, saves[n].datasize, saves[n].name, saves[n].comment, &saves[n].language, &saves[n].date, &saves[n].datasize, &saves[n].blocksize);
        if (result != 0)
        {
            sgc_core_error("Failed with %d %s", result, saves[n].filename);

            // BUGBUG: handle error conditions gracefully
            strncpy(saves[n].name, "Error", MAX_SAVE_FILENAME);
            continue;
        }
    }

	modeExit();

    return count;
}

// copies the specified MODE save game to the saveFileData buffer
int modeReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    int result = 0;

    if(backupDevice != MODEBackup)
    {
        return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("modeReadSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(outSize == 0 || outSize > MAX_SAVE_SIZE)
    {
        sgc_core_error("modeReadSaveFile: Save file size is invalid %d!!", outSize);
        return -2;
    }

    modeEnter();

    strcpy(tmpFilename, SaveDirectory);
    strcat(tmpFilename, "/");
    strcat(tmpFilename, filename);

    result = MODE_OpenFile(tmpFilename, 0);
    if(result != 0)
    {
        modeExit();
        sgc_core_error("modeReadSaveFile: Failed to open MODE file!!");
        return -2;
    }

    for(unsigned int bytesRead = 0; bytesRead < outSize; )
    {
        unsigned int count;

        count = MIN(outSize - bytesRead, 2048);

        MODE_ReadFile(SectorBuffer, bytesRead, 2048);

        for (unsigned int c = 0; c < count; ++c)
        {
            outBuffer[bytesRead + c] = SectorBuffer[c];
        }

        bytesRead += count;
    }

    MODE_CloseFile();

    modeExit();

    return 0;
}

// write the save game to the MODE
int modeWriteSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize)
{
    int result = 0;

    if(backupDevice != MODEBackup)
    {
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("modeWriteSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(inBuffer == NULL || filename == NULL)
    {
        sgc_core_error("modeWriteSaveFile: Save file size is invalid %d!!", inSize);
        return -2;
    }

    modeEnter();

    strcpy(tmpFilename, SaveDirectory);
    strcat(tmpFilename, "/");
    strcat(tmpFilename, filename);

    result = MODE_OpenFile(tmpFilename, 1);
    if (result != 0)
    {
        modeExit();
        sgc_core_error("modeWriteSaveFile: Failed to open MODE file for writing!!");
        return -2;
    }

    for(unsigned int bytesWritten = 0; bytesWritten < inSize; )
    {
        unsigned int count;

        count = MIN(inSize - bytesWritten, 2048);

        MODE_WriteFile(inBuffer + bytesWritten, bytesWritten, count);

        bytesWritten += count;
    }

    MODE_CloseFile();

    modeExit();

    return 0;
}

// delete the save
int modeDeleteSaveFile(int backupDevice, char* filename)
{
    if(backupDevice != MODEBackup)
    {
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("modeDeleteSaveFile: Filename is NULL!!");
        return -1;
    }

    modeEnter();

    strcpy(tmpFilename, SaveDirectory);
    strcat(tmpFilename, "/");
    strcat(tmpFilename, filename);

    MODE_DeleteFile(tmpFilename);

    modeExit();

    return 0;
}

int modeEnter(void)
{
    MODE_Open();

    return 0;
}

int modeExit(void)
{
    MODE_Close();

    return 0;
}

void MODE_WaitVSync(void)
{
    slSynch();
}

// read the bup header
int modeReadBUPHeader(char* filename, PBUP_HEADER bupHeader)
{
    int result = 0;

    if(!filename || !bupHeader)
    {
        return -1;
    }

    strcpy(tmpFilename, SaveDirectory);
    strcat(tmpFilename, "/");
    strcat(tmpFilename, filename);

    result = MODE_OpenFile(tmpFilename, 0);
    if(result != 0)
    {
        sgc_core_error("modeBUP: Failed to open MODE file!!");
        return -2;
    }

    //MODE always reads in sector chunks, so we would be overwritting the stack if reading directly to the header buffer
    MODE_ReadFile(SectorBuffer, 0, 2048);
    memcpy((unsigned char*)bupHeader, SectorBuffer, sizeof(BUP_HEADER));

    MODE_CloseFile();

    return 0;
}
