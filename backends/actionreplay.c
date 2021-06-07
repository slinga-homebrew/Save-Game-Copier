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

    // decompress the partition
    result = decompressPartition((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, &partitionBuf, &partitionSize);
    if(result != 0)
    {
        return result;
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
    result = decompressPartition((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, &partitionBuf, &partitionSize);
    if(result != 0)
    {
        return result;
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
    UNUSED_ARG(backupDevice);
    UNUSED_ARG(filename);
    UNUSED_ARG(inBuffer);
    UNUSED_ARG(inSize);

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
    PSAT_START_BLOCK_HEADER saveStartBlock = NULL;
    PSAT_BLOCK satBlocks = NULL;
    PRLE01_HEADER rleHeader = NULL;
    unsigned char* partitionBuf = NULL;
    unsigned int partitionSize = 0;
    unsigned char* block = NULL;
    unsigned char rleKey = 0;
    unsigned char* compressedBuf = NULL;
    unsigned int compressedSize = 0;
    int result = 0;

    if(backupDevice != ActionReplayBackup)
    {
        return -1;
    }

    if(filename == NULL)
    {
        sgc_core_error("actionReplayDeleteSaveFile: Filename is NULL!!");
        return -2;
    }

    //
    // decompress the Action Replay compressed save buffer
    //
    // decompress the partition
    result = decompressPartition((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), ACTION_REPLACE_SAVES_SIZE, &partitionBuf, &partitionSize);
    if(result != 0)
    {
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

    // get the SAT block table
    result = getSATBlocks(partitionBuf, partitionSize, ACTION_REPLAY_PARTITION_SIZE, saveStartBlock, &satBlocks);
    if(result < 0)
    {
        sgc_core_error("Failed to get SAT table");
        goto cleanup;
    }

    // iterate through the allocated blocks and zero them out
    while(satBlocks->blockNum)
    {
        block = (unsigned char*)(partitionBuf + (satBlocks->blockNum * ACTION_REPLAY_PARTITION_SIZE));

        // zero out each block
        memset(block, 0, ACTION_REPLAY_PARTITION_SIZE);

        ++satBlocks;
    }

    // compute the new RLE rleKey
    result = calcRLEKey(partitionBuf, partitionSize, &rleKey);
    if(result != 0)
    {
        sgc_core_error("Failed to calculate key!! %d", result);
        goto cleanup;
    }

    //
    // compress partition buffer
    //

    result = compressRLE01(rleKey, partitionBuf, partitionSize, NULL, &compressedSize);
    if(result != 0)
    {
        sgc_core_error("compress: %d\n", result);
        goto cleanup;
    }

    if(compressedSize > ACTION_REPLACE_SAVES_SIZE || compressedSize + sizeof(RLE01_HEADER) > ACTION_REPLACE_SAVES_SIZE)
    {
        sgc_core_error("compressSize too big: %x\n", compressedSize);
        result = -4;
        goto cleanup;
    }

    // we need room fo rthe RLE01 header
    compressedBuf = jo_malloc(compressedSize + sizeof(RLE01_HEADER));
    if(compressedBuf == NULL)
    {
        sgc_core_error("failed to alloc");
        result = -3;
        goto cleanup;
    }

    // set the header
    rleHeader = (PRLE01_HEADER)compressedBuf;
    memcpy(rleHeader->compressionMagic, RLE01_MAGIC, sizeof(rleHeader->compressionMagic));
    rleHeader->rleKey = rleKey;
    rleHeader->compressedSize = compressedSize;

    result = compressRLE01(rleKey, partitionBuf, partitionSize, compressedBuf + sizeof(RLE01_HEADER), &compressedSize);
    if(result != 0)
    {
        sgc_core_error("compress: %d", result);
        goto cleanup;
    }

    sgc_core_error("RLE key: %x", rleKey);
    sgc_core_error("comp: %x", compressedSize);

    // BUGBUG: this should be a cart specific write operation
    memcpy((unsigned char*)(CARTRIDGE_MEMORY + ACTION_REPLACE_SAVES_OFFSET), compressedBuf, compressedSize);

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

    if(compressedBuf)
    {
        jo_free(compressedBuf);
    }

    return result;
}

//
// Utility Functions
//

// Takes in a compressed buffer (including header) from an Action Replay cart
// On success dest contains the uncompressed buffer of destSize bytes
// Caller must free dest on success
// returns 0 on success, non-zero on failure
int decompressPartition(unsigned char *src, unsigned int srcSize, unsigned char **dest, unsigned int* destSize)
{
    PRLE01_HEADER header = NULL;
    int result = 0;

    if(src == NULL || srcSize == 0 || dest == NULL || destSize == NULL)
    {
        sgc_core_error("decomp: invalid args");
        return -1;
    }

    if(srcSize < sizeof(RLE01_HEADER))
    {
        sgc_core_error("decomp: invalid srcSize");
        return -2;
    }

    header = (PRLE01_HEADER)src;

    // must begin with "RLE01"
    // "DEF01" and "DEF02" are not supported
    if(memcmp(header->compressionMagic, RLE01_MAGIC, sizeof(header->compressionMagic)) != 0)
    {
        sgc_core_error("decomp: bad magic %c%c%c%c%c", header->compressionMagic[0], header->compressionMagic[1], header->compressionMagic[2], header->compressionMagic[3], header->compressionMagic[4]);
        return -3;
    }

    if(header->compressedSize >= srcSize || header->compressedSize < sizeof(RLE01_HEADER))
    {
        // we will read out of bounds
        sgc_core_error("decomp: compressed size");
        return -4;
    }

    //
    // decompress the Action Replay compressed save buffer
    //
    result = decompressRLE01(header->rleKey, src + sizeof(RLE01_HEADER), header->compressedSize - sizeof(RLE01_HEADER), NULL, destSize);
    if(result < 0)
    {
        sgc_core_error("Failed RLE01 %d", result);
        return -5;
    }

    *dest = jo_malloc(*destSize);
    if(*dest == NULL)
    {
        sgc_core_error("Failed to allocate %d", *destSize);
        return -6;
    }

    result = decompressRLE01(header->rleKey, src  + sizeof(RLE01_HEADER), header->compressedSize - sizeof(RLE01_HEADER), *dest, destSize);
    if(result < 0)
    {
        sgc_core_error("Failed 2 RLE01 %d", result);
        jo_free(*dest);
        return -7;
    }

    return 0;
}

// Decompresses RLE01 compressed buffer into dest
// To calculate number of bytes needed, set dest to NULL
// This function was reversed from function 0x002897dc in ARP_202C.BIN
int decompressRLE01(unsigned char rleKey, unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int* bytesNeeded)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;

    if(src == NULL || bytesNeeded == NULL)
    {
        return -1;
    }

    j = 0;
    i = 0;

    do
    {
        unsigned int count = 0;
        unsigned char val = 0;

        // three compressed cases
        // 1) not key
        // - copy the byte directly
        // - src + 1, dest + 1
        // 2) key followed by zero
        // -- copy key
        // -- src + 2, dest + 1
        // 3) key followed by non-zero, followed by val
        // -- copy val count times
        // -- src + 3, dest + count

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
        if (srcSize <= i)
        {
            if(bytesNeeded)
            {
                *bytesNeeded = j;
            }

            return 0;
        }
    } while(1);
}

// on success sets key to the least used byte in src
int calcRLEKey(unsigned char* src, unsigned int size, unsigned char* key)
{
    unsigned int* keyCounts = NULL;
    unsigned char val = 0;
    unsigned int minCount = -1;

    if(!src || !size || !key)
    {
        return -1;
    }

    keyCounts = (unsigned int*)jo_malloc(RLE01_MAX_COUNT * sizeof(unsigned int));
    if(keyCounts == NULL)
    {
        return -2;
    }
    memset(keyCounts, 0, RLE01_MAX_COUNT * sizeof(unsigned int));

    for(unsigned int i = 0; i < size; i++)
    {
        val = src[i];
        keyCounts[val]++;
    }

    for(unsigned int j = 0; j < RLE01_MAX_COUNT; j++)
    {
        if(keyCounts[j] < minCount)
        {
            // found less used key val
            *key = j;
            minCount = keyCounts[j];
        }

        if(minCount == 0)
        {
            // fast exit if a count of zero has been found
            break;
        }
    }

    if(keyCounts)
    {
        jo_free(keyCounts);
    }

    return 0;
}

// Compresses RLE01 compressed buffer into dest
// To calculate number of bytes needed, set dest to NULL
// This function was reversed from function 0x0028970e in ARP_202C.BIN
int compressRLE01(unsigned char rleKey, unsigned char *src, unsigned int srcSize, unsigned char *dest, unsigned int* bytesNeeded)
{
    unsigned int i = 0;
    unsigned int j = 0;

    if(src == NULL || bytesNeeded == NULL)
    {
        return -1;
    }

    j = 0;
    i = 0;

    do
    {
        // three compressed cases
        // 1) not key
        // - copy the byte directly
        // - src + 1, dest + 1
        // 2) key followed by zero
        // -- copy key
        // -- src + 2, dest + 1
        // 3) key followed by non-zero, followed by val
        // -- copy val count times
        // -- src + 3, dest + count

        unsigned char count = 0;
        unsigned char val = 0;

        val = src[i];

        // check how many times in a row curVal appears
        do
        {
            count++;
            if (*(src + i + count) != val)
            {
                break;
            }
        } while (count < RLE_MAX_REPEAT);

        // if count small, don't bother RLEing
        if(count < 4)
        {
            i++;

            if(dest)
            {
                dest[j] = val;
            }
            j++;

            if(val == rleKey)
            {
                dest[j] = 0;
                j++;
            }
        }
        else
        {
            if(srcSize < i + count)
            {
                count = srcSize - i;
            }

            if(dest)
            {
                dest[j] = rleKey;
            }
            j++;

            if(dest)
            {
                dest[j] = count;
            }
            j++;

            if(dest)
            {
                dest[j] = val;
            }
            j++;

            i += count;
        }

        // check if we are at the end
        if (srcSize <= i)
        {
            if(bytesNeeded)
            {
                *bytesNeeded = j;
            }

            return 0;
        }
    } while(1);
}
