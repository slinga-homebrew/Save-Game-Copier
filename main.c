// Save Game Copier - a utility that copies Sega Saturn save game files

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

#include <jo/jo.h>
#include "main.h"
#include "util.h"
#include "backup.h"

GAME g_Game = {0};
SAVES g_Saves[MAX_SAVES] = {0};

void jo_main(void)
{
    jo_core_init(JO_COLOR_Black);

    // check if internal memory is present
    // check if cartridge memory is present
    // check if external memory is present
    // check if satiator is present

    // allocate our save file buffer
    g_Game.saveFileData = jo_malloc(MAX_SAVE_SIZE);
    if(g_Game.saveFileData == NULL)
    {
        jo_core_error("Failed to allocated save file data buffer!!");
        return;
    }

    // increase the default heap size. LWRAM is not being used
    jo_add_memory_zone((unsigned char *)LWRAM, LWRAM_HEAP_SIZE);

    // ABC + start handler
    jo_core_set_restart_game_callback(abcStartHandler);

    // callbacks
    jo_core_add_callback(main_draw);
    jo_core_add_callback(main_input);

    jo_core_add_callback(listSaves_draw);
    jo_core_add_callback(listSaves_input);

    jo_core_add_callback(displaySave_draw);
    jo_core_add_callback(displaySave_input);

    jo_core_add_callback(format_draw);
    jo_core_add_callback(format_input);

    jo_core_add_callback(formatVerify_draw);
    jo_core_add_callback(formatVerify_input);

    jo_core_add_callback(collect_draw);
    jo_core_add_callback(collect_input);

    jo_core_add_callback(credits_draw);
    jo_core_add_callback(credits_input);

    // debug output
    //jo_core_add_callback(debugOutput_draw);

    // initial state
    transitionToState(STATE_MAIN);

    jo_core_run();
}

// useful debug output
void debugOutput_draw()
{
    jo_printf(2, 0, "Memory Usage: %d Frag: %d" , jo_memory_usage_percent(), jo_memory_fragmentation());
    jo_printf(2, 1, "Save: %d CO: %d" , g_Game.numSaves, g_Game.cursorOffset);

    /*
    if(jo_memory_fragmentation() > 100)
    {
        jo_core_error("High fragmentation!! %d", jo_memory_fragmentation());
        jo_reduce_memory_fragmentation();
        jo_core_error("High fragmentation!! %d", jo_memory_fragmentation());
    }
    */
}

// restarts the program if controller one presses ABC+Start
// this definitely leaks memory but whatever...
// maybe I should just call jo_main()?
void abcStartHandler(void)
{
    g_Game.input.pressedStartAC = true;
    g_Game.input.pressedB = true;
    transitionToState(STATE_MAIN);
    return;
}

// helper for transitioning between program states
// adds/removes callbacks as needed
// resets globals as needed
void transitionToState(int newState)
{
    clearScreen();

    g_Game.previousState = g_Game.state;

    switch(g_Game.state)
    {
        case STATE_UNINITIALIZED:
        case STATE_MAIN:
        case STATE_LIST_SAVES:
        case STATE_DISPLAY_SAVE:
        case STATE_FORMAT:
        case STATE_FORMAT_VERIFY:
        case STATE_COLLECT:
        case STATE_CREDITS:
            break;

        default:
            jo_core_error("%d is an invalid current state!!", g_Game.state);
            return;
    }

    switch(newState)
    {
        case STATE_MAIN:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = OPTIONS_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = MAIN_NUM_OPTIONS;
            break;

        case STATE_LIST_SAVES:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = OPTIONS_Y + 1;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = 0; // 0 options until we list the number of saves
            g_Game.numSaves = 0; // number of saves counted
            g_Game.listedSaves = false;
            break;

        case STATE_DISPLAY_SAVE:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = SAVES_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = SAVES_NUM_OPTIONS;
            g_Game.md5Calculated = false;
            g_Game.operationStatus = OPERATION_UNINIT;
            break;

        case STATE_FORMAT:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = OPTIONS_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = FORMAT_NUM_OPTIONS;
            g_Game.operationStatus = OPERATION_UNINIT;
            break;

        case STATE_FORMAT_VERIFY:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = VERIFY_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = FORMAT_VERIFY_NUM_OPTIONS;
            break;

        case STATE_COLLECT:
            break;

        case STATE_CREDITS:
            break;

        default:
            jo_core_error("%d is an invalid state!!", newState);
            return;
    }

    g_Game.state = newState;

    return;
}

// draws the main option screen
// Number of options are hardcoded
void main_draw(void)
{
    unsigned int y = 0;

    if(g_Game.state != STATE_MAIN)
    {
        return;
    }

    // heading
    jo_printf(HEADING_X, HEADING_Y + y++, "Save Game Copier Ver %s", VERSION);
    jo_printf(HEADING_X, HEADING_Y + y++, HEADING_UNDERSCORE);

    y = 0;

    // options
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Internal Memory");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Cartridge Memory");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "External Device (Floppy)");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Satiator");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "CD Memory");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Format Device");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Save Collect Project");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Credits");

    // cursor
    jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset, ">>");

    return;
}

// checks for up/down movement for the cursor
// erases the old one and draws a new one
// savesPage is set to true if the cursor is for the list saves page
void moveCursor(bool savesPage)
{
    int cursorOffset = g_Game.cursorOffset;
    int maxCursorOffset = g_Game.numStateOptions;

    // do nothing if there are no options
    // this can happen if we select a backup device with no saves for example
    if(g_Game.numStateOptions == 0)
    {
        return;
    }

    if (jo_is_pad1_key_pressed(JO_KEY_UP))
    {
        if(g_Game.input.pressedUp == false)
        {
            cursorOffset--;
        }
        g_Game.input.pressedUp = true;
    }
    else
    {
        g_Game.input.pressedUp = false;
    }

    if (jo_is_pad1_key_pressed(JO_KEY_DOWN))
    {
        if(g_Game.input.pressedDown == false)
        {
            cursorOffset++;
        }
        g_Game.input.pressedDown = true;
    }
    else
    {
        g_Game.input.pressedDown = false;
    }

    // if we moved the cursor, erase the old
    if(cursorOffset != g_Game.cursorOffset)
    {
        if(savesPage == true)
        {
            jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + (g_Game.cursorOffset % MAX_SAVES_PER_PAGE), "  ");
        }
        else
        {
            jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset, "  ");
        }
    }

    // validate the number of lives
    if(cursorOffset < 0)
    {
        cursorOffset = maxCursorOffset - 1;
    }

    if(cursorOffset >= maxCursorOffset)
    {
        cursorOffset = 0;
    }
    g_Game.cursorOffset = cursorOffset;
}

// handles input on the main screen
void main_input(void)
{
    if(g_Game.state != STATE_MAIN)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false)
        {
            g_Game.input.pressedStartAC = true;

            switch(g_Game.cursorOffset)
            {
                case MAIN_OPTION_INTERNAL:
                {
                    g_Game.backupDevice = JoInternalMemoryBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_CARTRIDGE:
                {
                    g_Game.backupDevice = JoCartridgeMemoryBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_EXTERNAL:
                {
                    g_Game.backupDevice = JoExternalDeviceBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_SATIATOR:
                {
                    g_Game.backupDevice = SatiatorBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_CD:
                {
                    g_Game.backupDevice = CdMemoryBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_FORMAT:
                {
                    transitionToState(STATE_FORMAT);
                    return;
                }
                case MAIN_OPTION_COLLECT:
                {
                    transitionToState(STATE_COLLECT);
                    return;
                }
                case MAIN_OPTION_CREDITS:
                {
                    transitionToState(STATE_CREDITS);
                    return;
                }
                default:
                {
                    jo_core_error("Invalid main option!!");
                    return;
                }
            }
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
    }

    // update the cursor
    moveCursor(false);
    return;
}

// draws the list saves screen
void listSaves_draw(void)
{
    int result = 0;

    if(g_Game.state != STATE_LIST_SAVES)
    {
        return;
    }

    result = getBackupDeviceName(g_Game.backupDevice, &g_Game.backupDeviceName);
    if(result != 0)
    {
        transitionToState(STATE_MAIN);
        return;
    }

    jo_printf(HEADING_X, HEADING_Y, "%s", g_Game.backupDeviceName);
    jo_printf(HEADING_X, HEADING_Y + 1, HEADING_UNDERSCORE);

    if(g_Game.listedSaves == false)
    {
        int count = 0;

        // BUP devices
        if(g_Game.backupDevice == JoInternalMemoryBackup ||
           g_Game.backupDevice == JoCartridgeMemoryBackup ||
           g_Game.backupDevice == JoExternalDeviceBackup ||
           g_Game.backupDevice == CdMemoryBackup ||
           g_Game.backupDevice == SatiatorBackup)
        {
            jo_memset(g_Saves, 0, sizeof(g_Saves));
            g_Game.listedSaves = true;

            // read the saves meta data
            count = listSaveFiles(g_Game.backupDevice, g_Saves, COUNTOF(g_Saves));
            if(count >= 0)
            {
                // update the count of saves
                g_Game.numSaves = count;
            }
            else
            {
                // something went wrong
                transitionToState(STATE_MAIN);
                return;
            }
        }
        else
        {
            // TODO: add Satiator support here
            g_Game.listedSaves = true;
            g_Game.numSaves = 0;
        }
    }

    if(g_Game.numSaves > 0)
    {
        int i = 0;
        int j = 0;

        // header
        jo_printf(OPTIONS_X, OPTIONS_Y, "%-11s %10s %6s", "Filename", "Bytes", "Blocks");

        // zero out the save print fields otherwise we will have stale data on the screen
        // when we go to other pages
        for(i = 0; i < MAX_SAVES_PER_PAGE; i++)
        {
            jo_printf(OPTIONS_X, OPTIONS_Y + i  + 1, "                                      ");
        }

        // print up to MAX_SAVES_PER_PAGE saves on the screen
        for(i = (g_Game.cursorOffset / MAX_SAVES_PER_PAGE) * MAX_SAVES_PER_PAGE, j = 0; i < g_Game.numSaves && j < MAX_SAVES_PER_PAGE; i++, j++)
        {
            jo_printf(OPTIONS_X, OPTIONS_Y + (i % MAX_SAVES_PER_PAGE) + 1, "%-11s %10d %6d", g_Saves[i].filename, g_Saves[i].datasize, g_Saves[i].blocksize);
        }

        g_Game.numStateOptions = g_Game.numSaves;
    }
    else
    {
        jo_printf(OPTIONS_X, OPTIONS_Y, "Found 0 saves on the device");
    }

    if(g_Game.numStateOptions > 0)
    {
        memcpy(g_Game.saveFilename, g_Saves[g_Game.cursorOffset].filename, MAX_SAVE_FILENAME);
        g_Game.saveFileSize = g_Saves[g_Game.cursorOffset].datasize;

        jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset % MAX_SAVES_PER_PAGE, ">>");
    }

    return;
}

// handles input on the list saves screen
// B returns to the main menu
void listSaves_input(void)
{
    if(g_Game.state != STATE_LIST_SAVES)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false)
        {
            g_Game.input.pressedStartAC = true;
            if(g_Game.numStateOptions == 0)
            {
                // there are 0 saves, just return back to the main screen
                transitionToState(STATE_MAIN);
                return;
            }

            transitionToState(STATE_DISPLAY_SAVE);
            return;
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
    }

    if(jo_is_pad1_key_pressed(JO_KEY_B))
    {
        if(g_Game.input.pressedB == false)
        {
            g_Game.input.pressedB = true;
            transitionToState(STATE_MAIN);
            return;
        }
    }
    else
    {
        g_Game.input.pressedB = false;
    }

    // update the cursor
    moveCursor(true);
    return;
}

// draws display save screen
void displaySave_draw(void)
{
    int result = 0;
    int y = 0;

    if(g_Game.state != STATE_DISPLAY_SAVE)
    {
        return;
    }

    jo_printf(HEADING_X, HEADING_Y, "Save Info");
    jo_printf(HEADING_X, HEADING_Y + 1, HEADING_UNDERSCORE);

    // only compute the MD5 hash once
    if(g_Game.md5Calculated == false)
    {
        result = readSaveFile(g_Game.backupDevice, g_Game.saveFilename, g_Game.saveFileData, g_Game.saveFileSize);
        if(result != 0)
        {
            jo_core_error("Failed to read the save!!");
            transitionToState(STATE_MAIN);
            return;
        }

        // print messages to the user so that can get an estimate of the time for longer operations
        result = calculateMD5Hash(g_Game.saveFileData, g_Game.saveFileSize, g_Game.md5Hash);
        if(result != 0)
        {
            // something went wrong
            transitionToState(STATE_MAIN);
            return;
        }

        g_Game.md5Calculated = true;
    }

    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Filename: %s        ", g_Game.saveFilename);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Device: %s        ", g_Game.backupDeviceName);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Address: 0x%08x-0x%08x        ", g_Game.saveFileData, g_Game.saveFileData + g_Game.saveFileSize);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Size: %d            ", g_Game.saveFileSize);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "MD5: %02x%02x%02x%02x%02x%02x%02x%02x", g_Game.md5Hash[0], g_Game.md5Hash[1], g_Game.md5Hash[2], g_Game.md5Hash[3], g_Game.md5Hash[4], g_Game.md5Hash[5], g_Game.md5Hash[6], g_Game.md5Hash[7]);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "     %02x%02x%02x%02x%02x%02x%02x%02x", g_Game.md5Hash[8], g_Game.md5Hash[9], g_Game.md5Hash[10], g_Game.md5Hash[11],  g_Game.md5Hash[12], g_Game.md5Hash[13], g_Game.md5Hash[14], g_Game.md5Hash[15]);

    y = 0;
    //y++;

    // options
    jo_printf(OPTIONS_X, SAVES_Y + y++, "Copy to Internal Memory");
    jo_printf(OPTIONS_X, SAVES_Y + y++, "Copy to Cartridge Memory");
    jo_printf(OPTIONS_X, SAVES_Y + y++, "Copy to External Device (Floppy)");
    jo_printf(OPTIONS_X, SAVES_Y + y++, "Copy to Satiator");
    jo_printf(OPTIONS_X, SAVES_Y + y++, "Delete Save");

    jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset, ">>");

    y++;

    // done formatting
    if(g_Game.operationStatus == OPERATION_SUCCESS)
    {
        y++;
        jo_printf(OPTIONS_X, SAVES_Y + y++, "Successfully performed operation.");
    }
    else if(g_Game.operationStatus == OPERATION_FAIL)
    {
        y++;
        jo_printf(OPTIONS_X, SAVES_Y + y++, "Failed to perform operation.");
    }

    return;
}

// handles input on the display saves screen
// B returns to the main menu
void displaySave_input(void)
{
    int result = 0;

    if(g_Game.state != STATE_DISPLAY_SAVE)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false)
        {
            g_Game.input.pressedStartAC = true;
            {
                switch(g_Game.cursorOffset)
                {
                    case SAVE_OPTION_INTERNAL:
                    {
                        result = writeSaveFile(JoInternalMemoryBackup, g_Game.saveFilename, g_Game.saveFileData, g_Game.saveFileSize);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_CARTRIDGE:
                    {
                        result = writeSaveFile(JoCartridgeMemoryBackup, g_Game.saveFilename, g_Game.saveFileData, g_Game.saveFileSize);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_EXTERNAL:
                    {
                        result = writeSaveFile(JoExternalDeviceBackup, g_Game.saveFilename, g_Game.saveFileData, g_Game.saveFileSize);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_SATIATOR:
                    {
                        result = writeSaveFile(SatiatorBackup, g_Game.saveFilename, g_Game.saveFileData, g_Game.saveFileSize);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_DELETE:
                    {
                        result = deleteSaveFile(g_Game.backupDevice, g_Game.saveFilename);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                }
            }
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
    }

    if(jo_is_pad1_key_pressed(JO_KEY_B))
    {
        if(g_Game.input.pressedB == false)
        {
            g_Game.input.pressedB = true;
            transitionToState(g_Game.previousState);
            return;
        }
    }
    else
    {
        g_Game.input.pressedB = false;
    }

    // update the cursor
    moveCursor(false);
    return;
}

// draws the format devices screen
// Number of options are hardcoded
void format_draw(void)
{
    unsigned int y = 0;

    if(g_Game.state != STATE_FORMAT)
    {
        return;
    }

    // heading
    jo_printf(HEADING_X, HEADING_Y + y++, "Format Device");
    jo_printf(HEADING_X, HEADING_Y + y++, HEADING_UNDERSCORE);

    y = 0;

    // options
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Internal Memory");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Cartridge Memory");
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "External Device (Floppy)");

    // cursor
    jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset, ">>");

    return;
}

// handles input on the format screen
void format_input(void)
{
    if(g_Game.state != STATE_FORMAT)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false)
        {
            g_Game.input.pressedStartAC = true;

            switch(g_Game.cursorOffset)
            {
                case MAIN_OPTION_INTERNAL:
                {
                    g_Game.backupDevice = JoInternalMemoryBackup;
                    transitionToState(STATE_FORMAT_VERIFY);
                    return;
                }
                case MAIN_OPTION_CARTRIDGE:
                {
                    g_Game.backupDevice = JoCartridgeMemoryBackup;
                    transitionToState(STATE_FORMAT_VERIFY);
                    return;
                }
                case MAIN_OPTION_EXTERNAL:
                {
                    g_Game.backupDevice = JoExternalDeviceBackup;
                    transitionToState(STATE_FORMAT_VERIFY);
                    return;
                }
            }
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
    }

    // did the player hit B
    if(jo_is_pad1_key_pressed(JO_KEY_B))
    {
        if(g_Game.input.pressedB == false)
        {
            g_Game.input.pressedB = true;

            transitionToState(STATE_MAIN);
            return;
        }
    }
    else
    {
        g_Game.input.pressedB = false;
    }

    // update the cursor
    moveCursor(false);
    return;

    // update the cursor
    moveCursor(false);
    return;
}

// draws the format devices screen
// Number of options are hardcoded
void formatVerify_draw(void)
{
    int result = 0;
    unsigned int y = 0;

    if(g_Game.state != STATE_FORMAT_VERIFY)
    {
        return;
    }

    result = getBackupDeviceName(g_Game.backupDevice, &g_Game.backupDeviceName);
    if(result != 0)
    {
        transitionToState(STATE_MAIN);
        return;
    }

    // heading
    jo_printf(HEADING_X, HEADING_Y + y++, "Format Device: %s", g_Game.backupDeviceName);
    jo_printf(HEADING_X, HEADING_Y + y++, HEADING_UNDERSCORE);
    y++;

    jo_printf(HEADING_X, HEADING_Y + y++, "WARNING: ALL SAVES WILL BE DELETED");
    jo_printf(HEADING_X, HEADING_Y + y++, "Are you sure:");

    y = 0;

    // options
    jo_printf(OPTIONS_X, VERIFY_Y + y++, "Yes");
    jo_printf(OPTIONS_X, VERIFY_Y + y++, "No");
    jo_printf(OPTIONS_X, VERIFY_Y + y++, "");

    // cursor
    jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset, ">>");

    // done formatting
    if(g_Game.operationStatus == OPERATION_SUCCESS)
    {
        y++;
        jo_printf(OPTIONS_X, VERIFY_Y + y++, "Successfully formatted device.");
    }
    else if(g_Game.operationStatus == OPERATION_FAIL)
    {
        y++;
        jo_printf(OPTIONS_X, VERIFY_Y + y++, "Failed to format device.");
    }

    return;
}

// handles input on the format verify screen
void formatVerify_input(void)
{
    int result = false;

    if(g_Game.state != STATE_FORMAT_VERIFY)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false)
        {
            g_Game.input.pressedStartAC = true;

            switch(g_Game.cursorOffset)
            {
                case VERIFY_YES:
                {
                    if(g_Game.operationStatus != OPERATION_UNINIT)
                    {
                        // we already formatted the device
                        transitionToState(g_Game.previousState);
                        return;
                    }

                    result = formatDevice(g_Game.backupDevice);
                    if(result != 0)
                    {
                        jo_core_error("Failed to format device!!");
                        g_Game.operationStatus = OPERATION_FAIL;
                        transitionToState(STATE_MAIN);
                        return;
                    }

                    g_Game.operationStatus = OPERATION_SUCCESS;
                    return;
                }
                case VERIFY_NO:
                {
                    transitionToState(g_Game.previousState);
                    return;
                }
            }
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
    }

    // did the player hit B
    if(jo_is_pad1_key_pressed(JO_KEY_B))
    {
        if(g_Game.input.pressedB == false)
        {
            g_Game.input.pressedB = true;

            transitionToState(g_Game.previousState);
            return;
        }
    }
    else
    {
        g_Game.input.pressedB = false;
    }

    // update the cursor
    moveCursor(false);
    return;
}

// draws the save games collect screen
void collect_draw(void)
{
    unsigned int y = 0;

    if(g_Game.state != STATE_COLLECT)
    {
        return;
    }

    // heading
    jo_printf(HEADING_X, HEADING_Y + y++, "Save Game Collect Project");
    jo_printf(HEADING_X, HEADING_Y + y++, HEADING_UNDERSCORE);
    y = 0;

    // message
    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "Want to share your saves with");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "others?");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "Send them to Cafe-Alpha:");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "ppcenter.webou.net/pskai/savedata/");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "Please append \".RAW\" to the save");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "filename before sending. ");

    return;
}

// handles input on the save game collect screen
// Any button press returns back to title screen
void collect_input(void)
{
    if(g_Game.state != STATE_COLLECT)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_B) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false &&
           g_Game.input.pressedB == false)
        {
            g_Game.input.pressedStartAC = true;
            g_Game.input.pressedB = true;
            transitionToState(STATE_MAIN);
            return;
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
        g_Game.input.pressedB = false;
    }
    return;
}

// draws the credits screen
void credits_draw(void)
{
    unsigned int y = 0;

    if(g_Game.state != STATE_CREDITS)
    {
        return;
    }

    // heading
    jo_printf(HEADING_X, HEADING_Y + y++, "Credits");
    jo_printf(HEADING_X, HEADING_Y + y++, HEADING_UNDERSCORE);
    y = 0;

    // message
    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1, "Special thanks to:");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "EmeraldNova for doing all Satiator");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "testing.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Antime, Ponut, VBT, and everyone");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "else at SegaXtreme keeping the");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Saturn dev scene alive.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Thank you to Takashi for the");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "original Save Game Copier idea");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "back in ~2002.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, " - Slinga");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "(github.com/slinga-homebrew)");

    return;
}

// handles input on the credits screen
// Any button press returns back to title screen
void credits_input(void)
{
    if(g_Game.state != STATE_CREDITS)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
       jo_is_pad1_key_pressed(JO_KEY_A) ||
       jo_is_pad1_key_pressed(JO_KEY_B) ||
       jo_is_pad1_key_pressed(JO_KEY_C))
    {
        if(g_Game.input.pressedStartAC == false &&
           g_Game.input.pressedB == false)
        {
            g_Game.input.pressedStartAC = true;
            g_Game.input.pressedB = true;
            transitionToState(STATE_MAIN);
            return;
        }
    }
    else
    {
        g_Game.input.pressedStartAC = false;
        g_Game.input.pressedB = false;
    }
    return;
}
