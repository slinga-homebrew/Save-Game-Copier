// Saturn Allocation Table (SAT) save partitions parsing
#pragma once

//
// SAT structures
//

//
// Saturn saves are stored in 64-byte blocks
// - the first 4 bytes of each block is a tag
// -- 0x80000000 = start of new save
// -- 0x00000000 = continuation block?
// - the next 30 bytes are metadata for the save (save name, language, comment, date, and size)
// -- the size is the size of the save data not counting the metadata
// - next is a variable array of 2-byte block ids. This array ends when 00 00 is encountered
// -- the block id for the 1st block (the one containing the metadata) is not present. In this case only 00 00 will be present
// -- the variable length array can be 0-512 elements (assuming max save is on the order of ~32k)
// -- to complicate matters, the block ids themselves can extend into multiple blocks. This makes computing the block count tricky
// - following the block ids is the save data itself
//

#define SAT_PARTITION_SIZE      0x40 // BUGBUG: this should be dynamic

#define SAT_MAX_SAVE_NAME       11
#define SAT_MAX_SAVE_COMMENT    10

#define SAT_START_BLOCK_TAG     0x80000000 // beginning of a save must start with this
#define SAT_CONTINUE_BLOCK_TAG  0x0        // all other blocks have a 0 tag


#define SAT_TAG_SIZE sizeof(((SAT_START_BLOCK_HEADER *)0)->tag)

// BUGBUG: get rid of this
#define SAT_BLOCK_USABLE_SIZE 0x40 - 4
#define SAT_BLOCK_HEADER_SIZE 0x1E

// struct at the beginning of a save block
#pragma pack(1)
typedef struct _SAT_START_BLOCK_HEADER
{
    unsigned int tag;
    char saveName[SAT_MAX_SAVE_NAME]; // not necessarily NULL terminated
    unsigned char language;
    char comment[SAT_MAX_SAVE_COMMENT]; // not necessarily NULL terminated
    unsigned int date;
    unsigned int saveSize; // in bytes
}SAT_START_BLOCK_HEADER, *PSAT_START_BLOCK_HEADER;
#pragma pack()


#define SAT_START_BLOCK_FLAG         0x1    // first block in the save. It contains SAT_START_BLOCK_HEADER followed by SAT table
#define SAT_TABLE_BLOCK_FLAG         0x2    // block contains the variable lenght SAT table
#define SAT_TABLE_END_BLOCK_FLAG     0x4    // block contains the end of the SAT table

// represents a SAT block. Used to find data within a SAT partition
typedef struct _SAT_BLOCK
{
    unsigned int blockNum;  // blockNum is the index into the partition. Multiply by block size
    unsigned int flags;     // it's possible for a block to have multiple flags at once
} SAT_BLOCK, *PSAT_BLOCK;


// parsing functions
int calcNumBlocks(unsigned int saveSize, unsigned int blockSize, unsigned int* numSaveBlocks);
int satListSaves(unsigned char* partitionBuffer, unsigned int partitionSize, unsigned int blockSize, PSAVES saves, unsigned int numSaves);
int getSaveStartBlock(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, char* saveName, PSAT_START_BLOCK_HEADER* metadata);
int getSATBlocks(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, PSAT_START_BLOCK_HEADER metadata, PSAT_BLOCK* satBlocks);
int readSATFromBlock(unsigned char* partitionBuf, unsigned int partitionSize, unsigned int blockSize, unsigned int currBlock, PSAT_BLOCK satBlocks, unsigned int maxBlocks, unsigned int* numBocks);
int getSATSave(unsigned char* partitionBuffer, unsigned int partitionSize, unsigned int blockSize, PSAT_BLOCK satTable, unsigned char* saveData, unsigned int saveSize);


