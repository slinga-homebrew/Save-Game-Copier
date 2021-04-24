// Action Replay Cartridge
#include "actionreplay.h"
#include "sat.h"

// returns true if the backup device is found
bool actionReplayIsBackupDeviceAvailable(int backupDevice)
{
    char* magic = NULL;

    if(backupDevice != ActionReplayBackup)
    {
        return false;
    }

    // check for the action replay signature
    magic = (char*)(CARTRIDGE_MEMORY + ACTION_REPLAY_MAGIC_OFFSET);

    if(strncmp(magic, ACTION_REPLAY_MAGIC, sizeof(ACTION_REPLAY_MAGIC) - 1) == 0)
    {
        return true;
    }

    return false;
}

// queries the saves on the Action Replay cartridge device and fills out the saves array
int actionReplayListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    unsigned char* partitionBuf = NULL;
    unsigned int partitionSize = 0;
    int foundSaves = 0;
    int result = 0;

    if(backupDevice != ActionReplayBackup)
    {
        sgc_core_error("Failed 1");
        return -1;
    }

    //
    // decompress the Action Replay compressed save buffer
    //
    result = decompressRLE01((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, NULL, &partitionSize);
    if(result < 0)
    {
        sgc_core_error("Failed RLE01 %d", result);
        return -2;
    }

    partitionBuf = jo_malloc(partitionSize);
    if(partitionBuf == NULL)
    {
        sgc_core_error("Failed to allocate %d", partitionSize);
        return -1;
    }

    result = decompressRLE01((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, partitionBuf, &partitionSize);
    if(result < 0)
    {
        sgc_core_error("Failed 2 RLE01 %d", result);
        goto cleanup;
    }

    // enumerate the saves
    foundSaves = satListSaves(partitionBuf, partitionSize, ACTION_REPLAY_PARTITION_SIZE, saves, numSaves);
    if(foundSaves < 0)
    {
        sgc_core_error("AR: failed to list %d\n", foundSaves);
        goto cleanup;
    }

cleanup:
    jo_free(partitionBuf);

    return foundSaves;
}

// copies the specified actionReplay save game to the saveFileData buffer
int actionReplayReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    PSAT_START_BLOCK_HEADER saveStartBlock = NULL;
    PSAT_BLOCK satBlocks = NULL;
    PBUP_HEADER bupHeader = NULL;
    unsigned char* partitionBuf = NULL;
    unsigned int partitionSize = 0;
    int result = 0;

    if(backupDevice != ActionReplayBackup)
    {
        return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("actionReplayReadSaveFile: Save file data buffer is NULL!!");
        return -1;
    }

    if(outSize < sizeof(BUP_HEADER) || outSize > (MAX_SAVE_SIZE + sizeof(BUP_HEADER)))
    {
        sgc_core_error("actionReplayReadSaveFile: Save file size is invalid %d!!", outSize);
        return -2;
    }
    bupHeader = (PBUP_HEADER)outBuffer;

    //
    // decompress the Action Replay compressed save buffer
    //
    result = decompressRLE01((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, NULL, &partitionSize);
    if(result < 0)
    {
        sgc_core_error("Failed RLE01 %d", result);
        return -2;
    }

    partitionBuf = jo_malloc(partitionSize);
    if(partitionBuf == NULL)
    {
        sgc_core_error("Failed to allocate %d", partitionSize);
        return -1;
    }

    result = decompressRLE01((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, partitionBuf, &partitionSize);
    if(result < 0)
    {
        sgc_core_error("Failed 2 RLE01 %d", result);
        goto cleanup;
    }

    //
    // Find the save, read it's SAT table, then read the save data
    //

    // find the start of the save block
    result = getSaveStartBlock(partitionBuf, partitionSize, ACTION_REPLAY_PARTITION_SIZE, filename, &saveStartBlock);
    if(result < 0)
    {
        sgc_core_error("Failed to find save!!\n");
        goto cleanup;
    }

    // set the bup header metadata
    memset(bupHeader, 0, sizeof(BUP_HEADER));

    memcpy(bupHeader->magic, VMEM_MAGIC_STRING, VMEM_MAGIC_STRING_LEN);
    strncpy((char*)bupHeader->dir.filename, saveStartBlock->saveName, SAT_MAX_SAVE_NAME);
    strncpy((char*)bupHeader->dir.comment, saveStartBlock->comment, SAT_MAX_SAVE_COMMENT);
    bupHeader->dir.language = saveStartBlock->language;
    bupHeader->dir.date = saveStartBlock->date;
    bupHeader->dir.datasize = saveStartBlock->saveSize;
    bupHeader->dir.blocksize = 0; // not needed
    bupHeader->date = saveStartBlock->date; // date is duplicated

    // validate the size
    if(saveStartBlock->saveSize > outSize - sizeof(BUP_HEADER))
    {
        sgc_core_error("Save size is too big!!\n");
        result = -1;
        goto cleanup;
    }

    // get the SAT block table
    result = getSATBlocks(partitionBuf, partitionSize, ACTION_REPLAY_PARTITION_SIZE, saveStartBlock, &satBlocks);
    if(result < 0)
    {
        sgc_core_error("Failed to get SAT table");
        goto cleanup;
    }

    // finally read the save data
    result = getSATSave(partitionBuf, partitionSize, ACTION_REPLAY_PARTITION_SIZE, satBlocks, outBuffer + sizeof(BUP_HEADER), saveStartBlock->saveSize);
    if(result < 0)
    {
        sgc_core_error("Failed to read save!!\n");
        goto cleanup;
    }

    result = 0;

cleanup:
    if(partitionBuf)
    {
        jo_free(partitionBuf);
    }

    if(satBlocks)
    {
        jo_free(satBlocks);
    }

    return result;
}

// write the save game to the actionReplay
int actionReplayWriteSaveFile(int backupDevice, char* filename, unsigned char* inBuffer, unsigned int inSize)
{
    UNUSED(backupDevice);
    UNUSED(filename);
    UNUSED(inBuffer);
    UNUSED(inSize);

    /*
    int result = 0;
    int fd = 0;

    if(backupDevice != actionReplayBackup)
    {
        return -1;
    }

    result = actionReplayEnter();
    if(result != 0)
    {
        sgc_core_error("Failed to detect actionReplay");
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("writeactionReplaySaveData: Save file data buffer is NULL!!");
        return -1;
    }

    if(inBuffer == NULL || filename == NULL)
    {
        sgc_core_error("writeactionReplaySaveData: Save file size is invalid %d!!", inSize);
        return -2;
    }

    fd = s_open(filename, FA_WRITE|FA_CREATE_ALWAYS);
    if(fd < 0)
    {
        sgc_core_error("writeactionReplaySaveData: Failed to open actionReplay file!!");
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
        sgc_core_error("writeactionReplaySaveData: Failed to read actionReplay file!!");
        return -3;
    }

    return 0;
    */

    return -1;

}

// delete the save
int actionReplayDeleteSaveFile(int backupDevice, char* filename)
{
    UNUSED(backupDevice);
    UNUSED(filename);

    /*
    int result = 0;

    if(backupDevice != actionReplayBackup)
    {
        return -1;
    }

    result = actionReplayEnter();
    if(result != 0)
    {
        sgc_core_error("Failed to detect actionReplay");
        //return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("deleteactionReplaySaveData: Filename is NULL!!");
        return -1;
    }

    result = s_unlink(filename);

    if(result < 0)
    {
        sgc_core_error("deleteactionReplaySaveData: Failed to read actionReplay file!!");
        return -3;
    }

    return 0;
    */

    return -1;
}

//
// Utility Functions
//

// Decompresses RLE01 compressed buffer into dest
// To calculate number of bytes needed, set dest to NULL
// This function was reversed from function 0x002897dc in ARP_202C.BIN
int decompressRLE01(unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int* bytesNeeded)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    unsigned char rleKey = 0;
    unsigned int compressedSize = 0;

    if(src == NULL || bytesNeeded == NULL)
    {
        return -1;
    }

    // dest can be NULL

    // must begin with "RLE01"
    if(memcmp(src, RLE01_MAGIC, sizeof(RLE01_MAGIC) - 1) != 0)
    {
        return -2;
    }

    // BUGBUG: bytes 0-4 should be RLE01
    // verify the signature before continuing

    rleKey = src[5];

    compressedSize = *(unsigned int*)&src[6];

    if(compressedSize >= srcSize)
    {
        // we will read out of bounds
        return -2;
    }

    //sgc_core_error("size is %x", compressedSize);

    j = 0;
    i = 10;

    do
    {
        unsigned int count = 0;
        unsigned char val = 0;

        if (src[i] == rleKey)
        {
            count = (int)(char)src[i + 1] & 0xff;
            if (count == 0)
            {
                if(dest)
                {
                    dest[j] = rleKey;
                }

                i = i + 2;
                goto continue_loop;
            }

            val = src[i + 2];
            i = i + 3;
            k = 0;

            if (count != 0)
            {
                do
                {
                    if(dest)
                    {
                        dest[j] = val;
                    }

                    j = j + 1;
                    k = k + 1;

                } while (k < count);
            }
        }
        else
        {
            if(dest)
            {
                dest[j] = src[i];
            }

            i = i + 1;
continue_loop:
            j = j + 1;
        }

        // check if we are at the end
        if (compressedSize <= i)
        {
            if(bytesNeeded)
            {
                *bytesNeeded = j;
            }

            return 0;
        }
    } while(1);
}

