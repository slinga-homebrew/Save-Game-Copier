#include "util.h"

// hackish way to to clear the screen
// Need to find API way
void clearScreen(void)
{
    for(int i = 0; i < 32; i++)
    {
        jo_printf(0, i, "                                          ");
    }
}

// calculates the MD5 hash of buffer
// md5Hash is an out parameter that must be at least MD5_HASH_SIZE (16) long
// returns 0 on success
int calculateMD5Hash(unsigned char* buffer, unsigned int bufferSize, unsigned char* md5Hash)
{
    MD5_CTX ctx = {0};

    if(md5Hash == NULL)
    {
        jo_core_error("Invalid parameters to calculateMD5Hash!!");
        return -1;
    }

    if(bufferSize == 0)
    {
        jo_memset(md5Hash, 0, bufferSize);
        return 0;
    }

    MD5_Init(&ctx);
    MD5_Update(&ctx, buffer, bufferSize);
    MD5_Final(md5Hash, &ctx);

    return 0;
}

