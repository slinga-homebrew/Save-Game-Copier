#pragma once

#include <jo/jo.h>
#include "md5/md5.h"

#define COUNTOF(x) sizeof(x)/sizeof(x[0])

#define LWRAM 0x00200000 // start of LWRAM memory. Doesn't appear to be used
#define LWRAM_HEAP_SIZE 0x100000 // number of bytes to extend heap by

#define JO_PRINTF_BUF_SIZE  (64)

// taken from Jo Engine core.h
// used to display an error message to the user
extern char __sgc_last_error[JO_PRINTF_BUF_SIZE];
void __sgc_core_error(char *message, const char *function);
# define sgc_core_error(...) do {sprintf(__sgc_last_error, __VA_ARGS__); __sgc_core_error(__sgc_last_error, __FUNCTION__);} while(0)

// This function prototype is not in jo/malloc.h
// Extend the heap
void jo_add_memory_zone(unsigned char *ptr, const unsigned int size_in_bytes);

// clears all the text on the screen
void clearScreen(void);

// calculates the MD5 hash of buffer
// md5Hash is an out parameter that must be at least MD5_HASH_SIZE (16) long
// returns 0 on success
int calculateMD5Hash(unsigned char* buffer, unsigned int bufferSize, unsigned char* md5Hash);
