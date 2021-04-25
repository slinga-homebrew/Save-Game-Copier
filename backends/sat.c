// Saturn Allocation Table (SAT) save partition parsing
#include <string.h>
#include "backend.h"
#include "sat.h"

// given a save size in bytes, calculate how many save blocks are required
// block size is the size of the block including the 4 byte tag field
int calcNumBlocks(unsigned int saveSize, unsigned int blockSize, unsigned int* numSaveBlocks)
{
    unsigned int totalBytes = 0;
    unsigned int fixedBytes = 0;
    unsigned int numBlocks = 0;
    unsigned int numBlocks2 = 0;

    if(saveSize == 0)
    {
        return -1;
    }

    if(numSaveBlocks == NULL)
    {
        return -2;
    }

    // block size must be 64-byte aligned
    if((blockSize % 0x40) != 0 || blockSize == 0)
    {
        return -3;
    }

    //
    // The stored save consists of:
    // - the save metadata header
    // - the variable length SAT table
    // - the save data itself
    //
    // What makes this tricky to compute is that the SAT table itself is stored
    // on the blocks
    //

    fixedBytes = sizeof(SAT_START_BLOCK_HEADER) - SAT_TAG_SIZE + saveSize;

    // calculate the number of SAT table entries required
    numBlocks = fixedBytes / (blockSize - SAT_TAG_SIZE);

    // +1 for 0x0000 SAT table terminator
    totalBytes = fixedBytes + ((numBlocks + 1) * sizeof(unsigned short));

    // we need to do this in a loop because it's possible that by adding a SAT
    // table entry we increased the number of bytes we need such that we need
    // yet another SAT table entry.
    do
    {
        numBlocks = numBlocks2;
        totalBytes = fixedBytes + ((numBlocks + 1) * sizeof(unsigned short));
        numBlocks2 = totalBytes / (blockSize - SAT_TAG_SIZE);

        if(totalBytes % (blockSize - SAT_TAG_SIZE))
        {
            numBlocks2++;
        }

    }while(numBlocks != numBlocks2);

    *numSaveBlocks = numBlocks;

    return 0;
}

// find all saves in the partition
int satListSaves(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, PSAVES saves, unsigned int numSaves)
{
    PSAT_START_BLOCK_HEADER metadata = NULL;
    unsigned int savesFound = 0;

    if(partitionBuf == NULL)
    {
        sgc_core_error("Buffer can't be NULL");
        return -1;
    }

    if(partitionSize == 0 || (partitionSize % blockSize) != 0)
    {
        sgc_core_error("Invalid partition size\n");
        return -2;
    }

    for(unsigned int i = 0; i < partitionSize; i += blockSize)
    {
        metadata = (PSAT_START_BLOCK_HEADER)(partitionBuf + i);

        // validate range
        if((unsigned char*)metadata < partitionBuf || (unsigned char*)metadata >= partitionBuf + partitionSize)
        {
            sgc_core_error("%x %x %x\n", partitionBuf, metadata, partitionSize);
            return -3;
        }

        // every save starts with a tag
        if(metadata->tag == SAT_START_BLOCK_TAG)
        {
            // save name
            // BUGBUG: figure out how to set filename (versus savename)
            memcpy(saves[savesFound].name, metadata->saveName, MAX_SAVE_FILENAME - 1);
            memcpy(saves[savesFound].filename, metadata->saveName, MAX_SAVE_FILENAME - 1);

            // langugae
            saves[savesFound].language = metadata->language;
            memcpy(saves[savesFound].comment, metadata->comment, MAX_SAVE_COMMENT - 1);
            saves[savesFound].date = metadata->date;
            saves[savesFound].datasize = metadata->saveSize;

            // blocksize isn't needed
            saves[savesFound].blocksize = 0;

            //sgc_core_error("%s %d %s %x %d", savename, language, comment, date, saveSize);

            savesFound++;

            // check if we are finished looking for saves
            if(savesFound >= numSaves)
            {
                // no more room in our saves array
                return savesFound;
            }
        }
    }

    return savesFound;
}

// locate a save start block based on save name
int getSaveStartBlock(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, char* saveName, PSAT_START_BLOCK_HEADER* metadata)
{
    if(partitionBuf == NULL)
    {
        return -1;
    }

    // block size must be 64-byte aligned
    if((blockSize % 0x40) != 0 || blockSize == 0)
    {
        return -2;
    }

    if(partitionSize == 0 || (partitionSize % blockSize) != 0)
    {
        return -3;
    }

    if(saveName == NULL || metadata == NULL)
    {
        return -4;
    }

    // parse through all blocks looking for a save start tag
    for(unsigned int i = 0; i < partitionSize; i += blockSize)
    {
        *metadata = (PSAT_START_BLOCK_HEADER)(partitionBuf + i);

        // start tag
        if((*metadata)->tag == SAT_START_BLOCK_TAG)
        {
            // found a save
            (*metadata)->date = (*metadata)->date;
            (*metadata)->saveSize = (*metadata)->saveSize;

            // found a save start block, check if it's for our game
            if(strncmp((*metadata)->saveName, saveName, SAT_MAX_SAVE_NAME) == 0)
            {
                return 0;
            }
        }
    }

    // save not found
    return -5;
}

// returns the SAT blocks on success. Must be freed by caller
int getSATBlocks(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, PSAT_START_BLOCK_HEADER metadata, PSAT_BLOCK* satBlocks)
{
    unsigned int numSatBlocks = 0;
    unsigned int writtenSatEntries = 0;
    unsigned int curSatTableIndex = 0;
    unsigned short firstEntry = 0;
    int result = 0;

    if(partitionBuf == NULL)
    {
        return -1;
    }

    // block size must be 64-byte aligned
    if((blockSize % 0x40) != 0 || blockSize == 0)
    {
        return -2;
    }

    if(partitionSize == 0 || (partitionSize % blockSize) != 0)
    {
        return -3;
    }

    if(metadata == NULL || satBlocks == NULL)
    {
        return -4;
    }

    result = calcNumBlocks(metadata->saveSize, blockSize, &numSatBlocks);
    if(result < 0)
    {
        return -5;
    }

    // +1 for the first SAT entry which isn't included
    numSatBlocks++;

    // check for integer overflow
    if(numSatBlocks > numSatBlocks * sizeof(SAT_BLOCK))
    {
        return -6;
    }

    *satBlocks = (PSAT_BLOCK)jo_malloc(numSatBlocks * sizeof(SAT_BLOCK));
    if(*satBlocks == NULL)
    {
        return -3;
    }
    memset(*satBlocks, 0, numSatBlocks * sizeof(SAT_BLOCK));

    // the first SAT entry isn't written in the SAT blocks array
    firstEntry = ((unsigned char*)metadata - partitionBuf)/SAT_PARTITION_SIZE;

    (*satBlocks)[0].blockNum = firstEntry;
    writtenSatEntries = 1;

    // loop through the blocks while we read the SAT table
    // this is tricky because we are writing to the satBlocks array while parsing it
    do
    {
        // make sure we aren't beyond the end of our array
        if(curSatTableIndex > writtenSatEntries)
        {
            jo_free(*satBlocks);
            *satBlocks = NULL;
            return -4;
        }

        // read SAT table entries from this block
        result = readSATFromBlock(partitionBuf, partitionSize, blockSize, curSatTableIndex, *satBlocks, numSatBlocks, &writtenSatEntries);
        if(result < 0)
        {
            jo_free(*satBlocks);
            *satBlocks = NULL;
            return -5;
        }

        curSatTableIndex++;

    }while(result != 0);

    // success
    return 0;
}

// reads the SAT table from satBlocks[currBlock]
int readSATFromBlock(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, unsigned int currBlock, PSAT_BLOCK satBlocks, unsigned int maxBlocks, unsigned int* numBlocks)
{
    PSAT_START_BLOCK_HEADER metadata = NULL;
    unsigned int startByte = 0;

    if(partitionBuf == NULL)
    {
        return -1;
    }

    // block size must be 64-byte aligned
    if((blockSize % 0x40) != 0 || blockSize == 0)
    {
        return -2;
    }

    if(partitionSize == 0 || (partitionSize % blockSize) != 0)
    {
        return -3;
    }

    if(satBlocks == NULL)
    {
        return -4;
    }

    if(currBlock >= *numBlocks)
    {
        return -5;
    }

    metadata = (PSAT_START_BLOCK_HEADER)(partitionBuf + (satBlocks[currBlock].blockNum * blockSize));

    // validate we are still in range
    if(metadata == NULL || (unsigned char*)metadata < partitionBuf || (unsigned char*)metadata >= partitionBuf + partitionSize)
    {
        return -6;
    }

    // first entry
    if(currBlock == 0)
    {
        // first block must have the start tag
        if(metadata->tag != SAT_START_BLOCK_TAG)
        {
            return -1;
        }

        // flags will be needed for retrieving save data
        satBlocks[currBlock].flags |= (SAT_START_BLOCK_FLAG | SAT_TABLE_BLOCK_FLAG);

        // where the block's data starts
        // first block has the PSAT_START_BLOCK_HEADER
        startByte = sizeof(SAT_START_BLOCK_HEADER);
    }
    else
    {
        // other blocks must not have the continuation tag
        if(metadata->tag != SAT_CONTINUE_BLOCK_TAG)
        {
            return -1;
        }

        // flags will be needed for retrieving save data
        satBlocks[currBlock].flags |= (SAT_TABLE_BLOCK_FLAG);

        // where the block's data starts
        // continuation block's only have a 4-byte tag field
        startByte = SAT_TAG_SIZE;
    }

    // loop through the block, recording SAT table entries until you find the
    // 0x0000 terminator or reach the end of the block
    for(startByte; startByte < blockSize; startByte += sizeof(unsigned short))
    {
        unsigned short index = *(unsigned short*)((unsigned char*)metadata + startByte);

        // found the last entry
        if(*numBlocks >= maxBlocks)
        {
            return -1;
        }

        // found a table entry, record it
        satBlocks[*numBlocks].blockNum = index;
        //sgc_core_error("%x", index);
        (*numBlocks)++;

        // end index
        if(index == 0)
        {
            // we are at the end index
            satBlocks[currBlock].flags |= SAT_TABLE_END_BLOCK_FLAG;
            return 0;
        }
    }

    // didn't find end val, continue checking
    return 1;
}

// given a SAT table, reads the save to save data
int getSATSave(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, PSAT_BLOCK satBlocks, unsigned char* saveData, unsigned int saveSize)
{
    unsigned int bytesWritten = 0;
    unsigned int bytesToCopy = 0;
    unsigned int blockDataSize = 0x40 -4;
    unsigned char* block = NULL;

    if(partitionBuf == NULL)
    {
        return -1;
    }

    // block size must be 64-byte aligned
    if((blockSize % 0x40) != 0 || blockSize == 0)
    {
        return -2;
    }

    if(partitionSize == 0 || (partitionSize % blockSize) != 0)
    {
        return -3;
    }

    if(satBlocks == NULL)
    {
        return -4;
    }

    if(saveData == NULL || saveSize == 0)
    {
        return -5;
    }

    // Edge cases
    // - last block isn't necessarily full, need save size for this
    // - first block contains header, sat table, and possibly data
    // - the last SAT table block can possibly include data

    while(satBlocks->blockNum)
    {
        block = (unsigned char*)(partitionBuf + (satBlocks->blockNum * blockSize));

        // validate we are still in range
        if(block < partitionBuf || block >= partitionBuf + partitionSize)
        {
            return -6;
        }

        // no flags means we are just a data block (no metadata, no SAT entries)
        if(satBlocks->flags == 0)
        {
            // check if we are the very last block and we aren't full
            if(saveSize - bytesWritten < blockDataSize)
            {
                // last block isn't full, copy less data
                bytesToCopy = saveSize - bytesWritten;
            }
            else
            {
                bytesToCopy = blockDataSize;
            }

            // copy the save bytes
            memcpy(saveData + bytesWritten, block + SAT_TAG_SIZE, bytesToCopy);
            bytesWritten += bytesToCopy;
        }

        // SAT table end means this block contains the last of the SAT blocks. It is possible there is save data here
        else if(satBlocks->flags & SAT_TABLE_END_BLOCK_FLAG)
        {
            unsigned int skipBytes = 0;

            // end of the SAT array, data can follow
            bytesToCopy = blockDataSize;

            // is this the start block?
            if(satBlocks->flags & SAT_START_BLOCK_FLAG)
            {
                // skip over the header data
                skipBytes += sizeof(SAT_START_BLOCK_HEADER) - SAT_TAG_SIZE;
            }

            // This is a SAT table block, parse until the 0x0000 and then start reading save bytes
            if(satBlocks->flags & SAT_TABLE_BLOCK_FLAG)
            {
                // skip over all SAT entries including the terminating 0x0000
                for(unsigned int i = skipBytes; i < blockDataSize; i += sizeof(unsigned short))
                {
                    unsigned short satIndex = *(unsigned short*)(block + i + SAT_TAG_SIZE);

                    if(satIndex == 0)
                    {
                        // found the 0s
                        skipBytes = i + sizeof(unsigned short);
                        break;
                    }
                }
            }

            bytesToCopy -= skipBytes;

            // check if we are the last very last block and we aren't full
            if(saveSize - bytesWritten < blockDataSize)
            {
                // last block isn't full, copy less data
                bytesToCopy = saveSize - bytesWritten;
            }

            // at maximum we can only copy blockDataSize at once
            if(bytesToCopy > blockDataSize)
            {
                return -1;
            }

            // another sanity check
            if(bytesWritten + bytesToCopy > saveSize)
            {
                return -1;
            }

            memcpy(saveData + bytesWritten, block + skipBytes + SAT_TAG_SIZE, bytesToCopy);
            bytesWritten += bytesToCopy;
        }

        ++satBlocks;
    }

    return 0;
}

