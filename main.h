// Save Game Extractor - a utility that modules Sega Saturn save game files over audio

/*
** Jo Sega Saturn Engine
** Copyright (c) 2012-2017, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Johannes Fetz nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL Johannes Fetz BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

// program version, keep this length to avoid having to resize strings
#define VERSION "3.2.2"

// program states
#define STATE_UNINITIALIZED      0
#define STATE_MAIN               1
#define STATE_LIST_SAVES         2
#define STATE_DISPLAY_SAVE       3
#define STATE_DISPLAY_MEMORY     4
#define STATE_DUMP_MEMORY        5
#define STATE_WRITE_MEMORY       6
#define STATE_FORMAT             7
#define STATE_FORMAT_VERIFY      8
#define STATE_COLLECT            9
#define STATE_CREDITS            10
#define STATE_PREVIOUS          -1 // go to the previous state

#define MAX_STATES              16 // how many states to record

// option selected on the main screen
#define MAIN_OPTION_INTERNAL     0
#define MAIN_OPTION_CARTRIDGE    1
#define MAIN_OPTION_EXTERNAL     2
#define MAIN_OPTION_SATIATOR     3
#define MAIN_OPTION_MODE         4
#define MAIN_OPTION_CD           5
#define MAIN_OPTION_DUMP_MEMORY  6
#define MAIN_OPTION_FORMAT       7
#define MAIN_OPTION_COLLECT      8
#define MAIN_OPTION_CREDITS      9
#define MAIN_OPTION_REBOOT      10

#define SAVE_OPTION_INTERNAL     0
#define SAVE_OPTION_CARTRIDGE    1
#define SAVE_OPTION_EXTERNAL     2
#define SAVE_OPTION_SATIATOR     3
#define SAVE_OPTION_WRITE_MEMORY 4
#define SAVE_OPTION_DELETE       5
#define SAVE_OPTION_MODE		 6

#define VERIFY_YES               0
#define VERIFY_NO                1

#define OPERATION_UNINIT         0
#define OPERATION_SUCCESS        1
#define OPERATION_FAIL           2

// position of the heading text
#define HEADING_X                2
#define HEADING_Y                2

#define OPTIONS_X                HEADING_X + 3
#define OPTIONS_Y                HEADING_Y + 4

#define VERIFY_X                 HEADING_X + 3
#define VERIFY_Y                 HEADING_Y + 7

#define SAVES_X                  HEADING_X + 3
#define SAVES_Y                  HEADING_Y + 11

#define CURSOR_X                 HEADING_X

#define MAIN_NUM_OPTIONS             9
#define FORMAT_NUM_OPTIONS           3
#define FORMAT_VERIFY_NUM_OPTIONS    2
#define SAVES_NUM_OPTIONS            6

#define HEADING_UNDERSCORE     "___________________________________"

#define MAX_SAVE_SIZE           (256 * 1024) // according to Cafe-Alpha this is the maximum size supported by the BIOS
#define MAX_SAVE_FILENAME       12
#define MAX_SAVES               255
#define MAX_SAVES_PER_PAGE      10

#define MD5_HASH_SIZE           16

// set this to 1 to skip device checks at boot. This will show the full menu
// set to 1 to skip
// BUGBUG: this should be a compile option,not a #define
#define SKIP_DEVICE_CHECKS      0

#define MAX_MENU_OPTIONS        12

// records whether or not an input has been pressed that frame
typedef struct _INPUTCACHE
{
    bool pressedUp;
    bool pressedDown;
    bool pressedLeft;
    bool pressedRight;
    bool pressedB;
    bool pressedStartAC;
    bool pressedLT;
    bool pressedRT;
} INPUTCACHE, *PINPUTCACHE;

// dynamic menu options
typedef struct _MENUOPTIONS
{
    char* optionText; // what to print to the user
    unsigned int option; // value of the option, used to detect which option was selected
} MENUOPTIONS, *PMENUOPTIONS;

typedef struct _GAME
{
    // game state variables
    int state; // current state of the program i.e. what screen it is in
    int previousStates[MAX_STATES]; // useful when going backwards from certain states
    int numStates;

    // number of options in the current state
    int numStateOptions;
    int numSaves; // number of saves on the device

    MENUOPTIONS menuOptions[MAX_MENU_OPTIONS];
    unsigned int numMenuOptions;

    // the position of the cursor in the 0 position
    // we add cursorOffset to get the correct position
    int cursorPosX;
    int cursorPosY;
    int cursorOffset;

    jo_backup_device backupDevice; // JoInternalMemoryBackup, JoCartridgeMemoryBackup, JoExternalDeviceBackup
    char* backupDeviceName;

    // flags for whether or not we found the specified backup device
    // this will help with our dynamic menu
    bool deviceInternalMemoryBackup;
    bool deviceCartridgeMemoryBackup;
    bool deviceExternalDeviceBackup;
    bool deviceSatiatorBackup;
    bool deviceCdMemoryBackup;
	bool deviceModeBackup;

    bool listedSaves; // set to true if we already queried the saves from the backup device

    int operationStatus; // status of the operation

    char saveFilename[MAX_SAVE_FILENAME]; // selected save file name
    unsigned int saveFileSize; // selected save file size
    unsigned char* saveFileData; // the raw data, points at transmissonFileData + TRANSMISSION_HEADER_SIZE

    unsigned int dumpMemoryAddress;
    unsigned int dumpMemorySize;

    bool md5Calculated; // set to true if we have calculated the md5 MD5_HASH_SIZE
    unsigned char md5Hash[MD5_HASH_SIZE];

    // hack to cache controller inputs
    INPUTCACHE input;

} GAME, *PGAME;

extern GAME g_Game;

// common functions
void jo_main(void);
void abcStartHandler(void);
void clearScreen(void);
int copyBIOS(unsigned int segment);
int copySaveFile(void);
void moveCursor(bool savesPage);
void moveDigitCursor(void);
void adjustHexValue(unsigned int* value, unsigned int digit, bool add);
void queryBackupDevices(void);

// menu options helpers
unsigned int initMenuOptions(int newState);

// state helper functions
void transitionToState(int newState);
void resetState(void);
int popState(void);
int pushState(int newState);

// main screen
void main_draw(void);
void main_input(void);

// list saves screen
void listSaves_draw(void);
void listSaves_input(void);

// playing save screen
void displaySave_draw(void);
void displaySave_input(void);

// dump bios screen
void dumpBios_draw(void);
void dumpBios_input(void);

// format screen
void format_draw(void);
void format_input(void);

// format verify screen
void formatVerify_draw(void);
void formatVerify_input(void);

// dump memory screen
void dumpMemory_draw(void);
void dumpMemory_input(void);

// save games collect projects screen
void collect_draw(void);
void collect_input(void);

// credits screen
void credits_draw(void);
void credits_input(void);

// debug output
void debugOutput_draw(void);

// function prototypes to suppress compiler warnings
void *memcpy(void *dest, const void *src, unsigned int n);
char *strncpy(char *dest, const char *src, unsigned int n);
int snprintf(char *str, unsigned int size, const char *format, ...);
