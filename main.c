// Save Game Copier - a utility that copies Sega Saturn save game files

/*
 * * Jo Sega Saturn Engine
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
#include "STDLIB.H"
#include "STRING.H"
#include "main.h"
#include "util.h"
#include "backends/backend.h"
#include "backends/satiator.h" // needed for satiatorReboot()

GAME g_Game = {0};
SAVES g_Saves[MAX_SAVES] = {0};

void jo_main(void)
{
    jo_core_init(JO_COLOR_Black);

    // check if internal memory is present
    // check if cartridge memory is present
    // check if external memory is present
    // check if satiator is present

    // allocate our save file buffe

    g_Game.saveBupHeader = jo_malloc(sizeof(BUP_HEADER) + MAX_SAVE_SIZE);
    if(g_Game.saveBupHeader == NULL)
    {
        sgc_core_error("Failed to allocated save file data buffer!!");
        return;
    }
    g_Game.saveFileData = (unsigned char*)(g_Game.saveBupHeader + 1);

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

    jo_core_add_callback(dumpMemory_draw);
    jo_core_add_callback(dumpMemory_input);

    jo_core_add_callback(collect_draw);
    jo_core_add_callback(collect_input);

    jo_core_add_callback(credits_draw);
    jo_core_add_callback(credits_input);

    // debug output
    //jo_core_add_callback(debugOutput_draw);

    queryBackupDevices();

    // initial state
    resetState();

    jo_core_run();
}

// useful debug output
void debugOutput_draw()
{
    // memory usage
    //jo_printf(2, 0, "Memory Usage: %d Frag: %d" , jo_memory_usage_percent(), jo_memory_fragmentation());
    //jo_printf(2, 1, "Save: %d CO: %d" , g_Game.numSaves, g_Game.cursorOffset);

    //// debug the state
    //jo_printf(2, 0, "State depth: %d      ", g_Game.numStates);
    //for(int i = 0; i < MAX_STATES; i++)
    //{
    //jo_printf(2 + (i*2), 1, "%d      ", g_Game.previousStates[i]);
    //}

    jo_printf(2, 25, "save name: %s", g_Game.saveName);
    jo_printf(2, 26, "file name: %s", g_Game.saveFilename);
}

// restarts the program if controller one presses ABC+Start
// this definitely leaks memory but whatever...
// maybe I should just call jo_main()?
void abcStartHandler(void)
{
    g_Game.input.pressedStartAC = true;
    g_Game.input.pressedB = true;
    resetState();
    return;
}

// verifies which backup devices are available
// needed for dynamic menu
void queryBackupDevices(void)
{
    if(SKIP_DEVICE_CHECKS == 0)
    {
        g_Game.deviceInternalMemoryBackup = isBackupDeviceAvailable(JoInternalMemoryBackup);
        g_Game.deviceCartridgeMemoryBackup = isBackupDeviceAvailable(JoCartridgeMemoryBackup);
        g_Game.deviceExternalDeviceBackup = isBackupDeviceAvailable(JoExternalDeviceBackup);
        g_Game.deviceActionReplayBackup = isBackupDeviceAvailable(ActionReplayBackup);
        g_Game.deviceSatiatorBackup = isBackupDeviceAvailable(SatiatorBackup);
        g_Game.deviceCdMemoryBackup = isBackupDeviceAvailable(CdMemoryBackup);

        // some Satiator users were reporting black screens at boot
        // possibly related to MODE?
        if(g_Game.deviceSatiatorBackup == false)
        {
            g_Game.deviceModeBackup = isBackupDeviceAvailable(MODEBackup);
        }
    }
    else
    {
        g_Game.deviceInternalMemoryBackup = true;
        g_Game.deviceCartridgeMemoryBackup = true;
        g_Game.deviceExternalDeviceBackup = true;
        g_Game.deviceActionReplayBackup = true;
        g_Game.deviceSatiatorBackup = true;
        g_Game.deviceCdMemoryBackup = true;
        g_Game.deviceModeBackup = true;
    }

    return;
}

// helper for transitioning between program states
// adds/removes callbacks as needed
// resets globals as needed
void transitionToState(int newState)
{
    clearScreen();

    // STATE_PREVIOUS is a handled specially
    // one pop for our current state
    // the second pop to get to our previous state
    if(newState == STATE_PREVIOUS)
    {
        newState = popState();
        newState = popState();
    }

    // save off the new state we are in
    pushState(newState);

    switch(g_Game.state)
    {
        case STATE_UNINITIALIZED:
        case STATE_MAIN:
        case STATE_LIST_SAVES:
        case STATE_DISPLAY_SAVE:
        case STATE_DISPLAY_MEMORY:
        case STATE_FORMAT:
        case STATE_FORMAT_VERIFY:
        case STATE_DUMP_MEMORY:
        case STATE_WRITE_MEMORY:
        case STATE_COLLECT:
        case STATE_CREDITS:
            break;

        default:
            sgc_core_error("%d is an invalid current state!!", g_Game.state);
            return;
    }

    switch(newState)
    {
        case STATE_MAIN:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = OPTIONS_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = initMenuOptions(STATE_MAIN);
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
        case STATE_DISPLAY_MEMORY:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = SAVES_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = SAVES_NUM_OPTIONS;
            g_Game.md5Calculated = false;
            g_Game.operationStatus = OPERATION_UNINIT;
            g_Game.numStateOptions = initMenuOptions(STATE_DISPLAY_SAVE);
            break;

        case STATE_FORMAT:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = OPTIONS_Y;
            g_Game.cursorOffset = 0;
            g_Game.operationStatus = OPERATION_UNINIT;
            g_Game.numStateOptions = initMenuOptions(STATE_FORMAT);
            break;

        case STATE_FORMAT_VERIFY:
            g_Game.cursorPosX = CURSOR_X;
            g_Game.cursorPosY = VERIFY_Y;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = FORMAT_VERIFY_NUM_OPTIONS;
            break;

        case STATE_DUMP_MEMORY:
            g_Game.cursorPosX = CURSOR_X + 14;
            g_Game.cursorPosY = VERIFY_Y - 2;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = 16;
            g_Game.dumpMemoryAddress = 0;
            g_Game.dumpMemorySize = 0;
            break;

        case STATE_WRITE_MEMORY:
            g_Game.cursorPosX = CURSOR_X + 14;
            g_Game.cursorPosY = VERIFY_Y - 2;
            g_Game.cursorOffset = 0;
            g_Game.numStateOptions = 16;
            g_Game.dumpMemoryAddress = 0;
            g_Game.dumpMemorySize = g_Game.saveFileSize;
            g_Game.operationStatus = OPERATION_UNINIT;
            break;

        case STATE_COLLECT:
            break;

        case STATE_CREDITS:
            break;

        default:
            sgc_core_error("%d is an invalid state!!", newState);
            return;
    }

    g_Game.state = newState;

    return;
}

// reset the global state
// useful when a major error happens
void resetState(void)
{
    g_Game.numStates = 0;
    memset(g_Game.previousStates, 0, sizeof(g_Game.previousStates));
    transitionToState(STATE_MAIN);
}

// pop off the current state
int popState(void)
{
    int tempState;

    if(g_Game.numStates > 0)
    {
        g_Game.numStates--;
        tempState = g_Game.previousStates[g_Game.numStates];
        g_Game.previousStates[g_Game.numStates] = 0;
        return tempState;
    }
    else
    {
        // should never get here
        sgc_core_error("popState: invalid numStates %d!!", g_Game.numStates);
        g_Game.numStates = 0;
        return STATE_MAIN;
    }
}

// push a state to our array
int pushState(int newState)
{
    if(g_Game.numStates + 1 >= MAX_STATES)
    {
        // bugbug: should never get here too deep
        // should never get here
        sgc_core_error("pushState: invalid numStates %d!!", g_Game.numStates);
        g_Game.numStates = 0;
        return STATE_MAIN;
    }

    g_Game.previousStates[g_Game.numStates] = newState;
    g_Game.numStates++;

    return 0;
}

// initialize the menu for the specified state
// the dynamic menu should only show options that make sense for the user
// ex. hide Satiator for non-Satiator users etc
// returns the number of options on success
unsigned int initMenuOptions(int newState)
{
    unsigned int numMenuOptions = 0;

    memset(&g_Game.menuOptions, 0, sizeof(g_Game.menuOptions));
    g_Game.numMenuOptions = 0;

    switch(newState)
    {
        case STATE_MAIN:
        {
            // only show main menu options for devices that have been detected
            if(g_Game.deviceInternalMemoryBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Internal Memory";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_INTERNAL;
                numMenuOptions++;
            }

            if(g_Game.deviceCartridgeMemoryBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Cartridge Memory";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_CARTRIDGE;
                numMenuOptions++;
            }

            if(g_Game.deviceExternalDeviceBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "External Device (Floppy)";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_EXTERNAL;
                numMenuOptions++;
            }

            if(g_Game.deviceActionReplayBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Action Replay (Beta Read-Only)";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_ACTION_REPLAY;
                numMenuOptions++;
            }

            if(g_Game.deviceSatiatorBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Satiator";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_SATIATOR;
                numMenuOptions++;
            }

            if (g_Game.deviceModeBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "MODE";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_MODE;
                numMenuOptions++;
            }

            if(g_Game.deviceCdMemoryBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "CD File System";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_CD;
                numMenuOptions++;
            }

            g_Game.menuOptions[numMenuOptions].optionText = "Dump Memory";
            g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_DUMP_MEMORY;
            numMenuOptions++;

            if(g_Game.deviceInternalMemoryBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Format Device";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_FORMAT;
                numMenuOptions++;
            }

            g_Game.menuOptions[numMenuOptions].optionText = "Save Collect Project";
            g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_COLLECT;
            numMenuOptions++;

            g_Game.menuOptions[numMenuOptions].optionText = "Credits";
            g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_CREDITS;
            numMenuOptions++;

            g_Game.menuOptions[numMenuOptions].optionText = "Exit to CD Player";
            g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_EXIT;
            numMenuOptions++;

            if(g_Game.deviceSatiatorBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Exit to Satiator";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_EXIT_SATIATOR;
                numMenuOptions++;
            }

            g_Game.menuOptions[numMenuOptions].optionText = "Reboot";
            g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_REBOOT;
            numMenuOptions++;

            break;
        }

        case STATE_DISPLAY_SAVE:
        {
            // only show display save menu options for devices that have been detected
            // don't allow user to copy file back to same device
            // don't allow user to delete CD save
            if(g_Game.deviceInternalMemoryBackup == true && g_Game.backupDevice != JoInternalMemoryBackup)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Copy to Internal Memory";
                g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_INTERNAL;
                numMenuOptions++;
            }

            if(g_Game.deviceCartridgeMemoryBackup == true && g_Game.backupDevice != JoCartridgeMemoryBackup)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Copy to Cartridge Memory";
                g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_CARTRIDGE;
                numMenuOptions++;
            }

            if(g_Game.deviceExternalDeviceBackup == true && g_Game.backupDevice != JoExternalDeviceBackup)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Copy to External Device (Floppy)";
                g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_EXTERNAL;
                numMenuOptions++;
            }

            if(g_Game.deviceSatiatorBackup == true && g_Game.backupDevice != SatiatorBackup)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Copy to Satiator";
                g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_SATIATOR;
                numMenuOptions++;
            }

            if (g_Game.deviceModeBackup == true && g_Game.backupDevice != MODEBackup)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Copy to MODE";
                g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_MODE;
                numMenuOptions++;
            }

            // TODO: temporarily disable write to memory option
            //g_Game.menuOptions[numMenuOptions].optionText = "Write to Memory";
            //g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_WRITE_MEMORY;
            //numMenuOptions++;

            // this check should really be isBackupDeviceWriteable()
            // for now we just check if it's not the cd
            if(g_Game.backupDevice != CdMemoryBackup && g_Game.backupDevice != MemoryBackup && g_Game.backupDevice != ActionReplayBackup)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Delete Save";
                g_Game.menuOptions[numMenuOptions].option = SAVE_OPTION_DELETE;
                numMenuOptions++;
            }
            break;
        }

        case STATE_FORMAT:
            // only show format menu options for devices that have been detected
            if(g_Game.deviceInternalMemoryBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Internal Memory";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_INTERNAL;
                numMenuOptions++;
            }

            if(g_Game.deviceCartridgeMemoryBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "Cartridge Memory";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_CARTRIDGE;
                numMenuOptions++;
            }

            if(g_Game.deviceExternalDeviceBackup == true)
            {
                g_Game.menuOptions[numMenuOptions].optionText = "External Device (Floppy)";
                g_Game.menuOptions[numMenuOptions].option = MAIN_OPTION_EXTERNAL;
                numMenuOptions++;
            }
            break;

        default:
            sgc_core_error("%d is an invalid current state!!", g_Game.state);
            resetState();
            return 0;
    }

    g_Game.numMenuOptions = numMenuOptions;
    return numMenuOptions;
}

// helper function to take the cursor index and return the value
// of the menu option
unsigned int getMenuOptionByIndex(unsigned int index)
{
    if(index >= g_Game.numMenuOptions)
    {
        sgc_core_error("Invalid index %d for menu options (%d)!!", index, g_Game.numMenuOptions);
        resetState();
        return -1;
    }

    return g_Game.menuOptions[index].option;
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
    for(unsigned int i = 0; i < g_Game.numMenuOptions; i++)
    {
        jo_printf(OPTIONS_X, OPTIONS_Y + i, g_Game.menuOptions[i].optionText);
    }

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

    //
    // check for up/down which moves up or down the list by 1
    //
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

    //
    // check for left/right which moves up or down the list by a page
    //
    if (jo_is_pad1_key_pressed(JO_KEY_LEFT))
    {
        if(g_Game.input.pressedLeft == false)
        {
            cursorOffset -= MAX_SAVES_PER_PAGE;
        }
        g_Game.input.pressedLeft = true;
    }
    else
    {
        g_Game.input.pressedLeft = false;
    }

    if (jo_is_pad1_key_pressed(JO_KEY_RIGHT))
    {
        if(g_Game.input.pressedRight == false)
        {
            cursorOffset += MAX_SAVES_PER_PAGE;
        }
        g_Game.input.pressedRight = true;
    }
    else
    {
        g_Game.input.pressedRight = false;
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
    unsigned int option;

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

            option = getMenuOptionByIndex(g_Game.cursorOffset);

            switch(option)
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
                case MAIN_OPTION_ACTION_REPLAY:
                {
                    g_Game.backupDevice = ActionReplayBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_SATIATOR:
                {
                    g_Game.backupDevice = SatiatorBackup;
                    transitionToState(STATE_LIST_SAVES);
                    return;
                }
                case MAIN_OPTION_MODE:
                {
                    g_Game.backupDevice = MODEBackup;
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
                case MAIN_OPTION_DUMP_MEMORY:
                {
                    g_Game.backupDevice = MemoryBackup;
                    transitionToState(STATE_DUMP_MEMORY);
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
                case MAIN_OPTION_EXIT:
                {
                    jo_core_exit_to_multiplayer();
                    return;
                }
                case MAIN_OPTION_EXIT_SATIATOR:
                {
                    satiatorReboot();
                    return;
                }
                case MAIN_OPTION_REBOOT:
                {
                    jo_core_restart_saturn();
                    return;
                }
                default:
                {
                    sgc_core_error("Invalid main option!!");
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

int compareSaveName (const void * a, const void * b)
{
    PSAVES aSave = (PSAVES)a;
    PSAVES bSave = (PSAVES)b;

    return strcmp(aSave->name, bSave->name);
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
        transitionToState(STATE_PREVIOUS);
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
            g_Game.backupDevice == SatiatorBackup ||
            g_Game.backupDevice == ActionReplayBackup ||
            g_Game.backupDevice == MODEBackup)
        {
            jo_memset(g_Saves, 0, sizeof(g_Saves));
            g_Game.listedSaves = true;

            // read the saves meta data
            count = listSaveFiles(g_Game.backupDevice, g_Saves, COUNTOF(g_Saves));
            if(count >= 0)
            {

                // sort the saves here
                qsort(g_Saves, count, sizeof(g_Saves[0]), compareSaveName);

                // update the count of saves
                g_Game.numSaves = count;
            }
            else
            {
                // something went wrong
                transitionToState(STATE_PREVIOUS);
                return;
            }
        }
        else
        {
            // TODO: I forgot why this was here >_<
            g_Game.listedSaves = true;
            g_Game.numSaves = 0;
        }
    }

    if(g_Game.numSaves > 0)
    {
        int i = 0;
        int j = 0;

        // header
        jo_printf(OPTIONS_X, OPTIONS_Y, "%-11s  %-10s  %6s", "Save Name", "Comment", "Bytes");

        // zero out the save print fields otherwise we will have stale data on the screen
        // when we go to other pages
        for(i = 0; i < MAX_SAVES_PER_PAGE; i++)
        {
            jo_printf(OPTIONS_X, OPTIONS_Y + i  + 1, "                                      ");
        }

        // print up to MAX_SAVES_PER_PAGE saves on the screen
        for(i = (g_Game.cursorOffset / MAX_SAVES_PER_PAGE) * MAX_SAVES_PER_PAGE, j = 0; i < g_Game.numSaves && j < MAX_SAVES_PER_PAGE; i++, j++)
        {
            jo_printf(OPTIONS_X, OPTIONS_Y + (i % MAX_SAVES_PER_PAGE) + 1, "%-11s  %-10s  %6d", g_Saves[i].name, g_Saves[i].comment, g_Saves[i].datasize);
        }

        g_Game.numStateOptions = g_Game.numSaves;
    }
    else
    {
        jo_printf(OPTIONS_X, OPTIONS_Y, "Found 0 saves on the device");
    }

    if(g_Game.numStateOptions > 0)
    {
        // copy the save data
        strncpy(g_Game.saveFilename, g_Saves[g_Game.cursorOffset].filename, MAX_FILENAME);
        g_Game.saveFilename[MAX_FILENAME - 1] = '\0';

        strncpy(g_Game.saveName, g_Saves[g_Game.cursorOffset].name, MAX_SAVE_FILENAME);
        g_Game.saveName[MAX_SAVE_FILENAME - 1] = '\0';

        strncpy(g_Game.saveComment, g_Saves[g_Game.cursorOffset].comment, MAX_SAVE_COMMENT);
        g_Game.saveComment[MAX_SAVE_COMMENT - 1] = '\0';

        g_Game.saveLanguage = g_Saves[g_Game.cursorOffset].language;
        g_Game.saveDate = g_Saves[g_Game.cursorOffset].date;
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
                transitionToState(STATE_PREVIOUS);
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
            transitionToState(STATE_PREVIOUS);
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

// draws the display save screen and the display memory screen
void displaySave_draw(void)
{
    int result = 0;
    int y = 0;
    unsigned char* saveFileData = NULL;
    unsigned int saveFileSize = 0;
    jo_backup_date jo_date = {0};

    if(g_Game.state != STATE_DISPLAY_SAVE && g_Game.state != STATE_DISPLAY_MEMORY)
    {
        return;
    }

    if(g_Game.state == STATE_DISPLAY_SAVE)
    {
        saveFileData = (unsigned char*)g_Game.saveBupHeader  + sizeof(BUP_HEADER);
        saveFileSize =  g_Game.saveFileSize;
    }
    else
    {
        saveFileData = (unsigned char*)g_Game.dumpMemoryAddress;
        saveFileSize =  g_Game.dumpMemorySize;
    }

    // heading
    if(g_Game.state == STATE_DISPLAY_SAVE)
    {
        jo_printf(HEADING_X, HEADING_Y, "Save Info");
    }
    else
    {
        jo_printf(HEADING_X, HEADING_Y, "Memory Info");
    }
    jo_printf(HEADING_X, HEADING_Y + 1, HEADING_UNDERSCORE);

    // only compute the MD5 hash once
    if(g_Game.md5Calculated == false)
    {
        if(g_Game.state == STATE_DISPLAY_SAVE)
        {
            result = readSaveFile(g_Game.backupDevice, g_Game.saveFilename, (unsigned char*)g_Game.saveBupHeader, g_Game.saveFileSize + sizeof(BUP_HEADER));
            if(result != 0)
            {
                sgc_core_error("Failed to read the save!!");
                transitionToState(STATE_PREVIOUS);
                return;
            }
        }

        // print messages to the user so that can get an estimate of the time for longer operations
        result = calculateMD5Hash(saveFileData, saveFileSize, g_Game.md5Hash);
        if(result != 0)
        {
            // something went wrong
            transitionToState(STATE_PREVIOUS);
            return;
        }

        result = getBackupDeviceName(g_Game.backupDevice, &g_Game.backupDeviceName);
        if(result != 0)
        {
            transitionToState(STATE_PREVIOUS);
            return;
        }

        g_Game.md5Calculated = true;
    }

    // convert between internal date and jo_date
    bup_getdate(g_Game.saveDate, &jo_date);

    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Filename: %s        ", g_Game.saveFilename);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Device: %s        ", g_Game.backupDeviceName);
    y++;

    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Save Name: %s        ", g_Game.saveName);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Comment: %s         ", g_Game.saveComment);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Date: %d/%d/%d %d:%d         ", jo_date.month, jo_date.day, jo_date.year + 1980, jo_date.time, jo_date.min);
    y++;

    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Size: %d            ", g_Game.saveFileSize);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Address: 0x%08x-0x%08x        ", saveFileData, saveFileData + saveFileSize);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "MD5: %02x%02x%02x%02x%02x%02x%02x%02x", g_Game.md5Hash[0], g_Game.md5Hash[1], g_Game.md5Hash[2], g_Game.md5Hash[3], g_Game.md5Hash[4], g_Game.md5Hash[5], g_Game.md5Hash[6], g_Game.md5Hash[7]);
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "     %02x%02x%02x%02x%02x%02x%02x%02x", g_Game.md5Hash[8], g_Game.md5Hash[9], g_Game.md5Hash[10], g_Game.md5Hash[11],  g_Game.md5Hash[12], g_Game.md5Hash[13], g_Game.md5Hash[14], g_Game.md5Hash[15]);

    y = 0;

    // options
    for(unsigned int i = 0; i < g_Game.numMenuOptions; i++)
    {
        jo_printf(OPTIONS_X, SAVES_Y + i, g_Game.menuOptions[i].optionText);
        y++; // spacing for the error messages
    }

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
        jo_printf(OPTIONS_X, SAVES_Y + y++, "Failed to copy the save, ");
        jo_printf(OPTIONS_X, SAVES_Y + y++, "possibly due to a lack of space.  ");
    }
    else if(g_Game.operationStatus == OPERATION_FAIL_DELETE)
    {
        y++;
        jo_printf(OPTIONS_X, SAVES_Y + y++, "Failed to perform operation.     ");
    }

    return;
}

// handles input on the display saves and display memory screen
// B returns to the main menu
void displaySave_input(void)
{
    int result = 0;
    unsigned int option = 0;
    unsigned char* saveFileData = NULL;
    unsigned int saveFileSize = 0;

    if(g_Game.state != STATE_DISPLAY_SAVE && g_Game.state != STATE_DISPLAY_MEMORY)
    {
        return;
    }

    if(g_Game.state == STATE_DISPLAY_SAVE)
    {
        saveFileData = (unsigned char*)g_Game.saveBupHeader;
        saveFileSize = g_Game.saveFileSize + sizeof(BUP_HEADER);
    }
    else
    {
        saveFileData = (unsigned char*)g_Game.dumpMemoryAddress;
        saveFileSize = g_Game.dumpMemorySize;
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
                option = getMenuOptionByIndex(g_Game.cursorOffset);

                switch(option)
                {
                    case SAVE_OPTION_INTERNAL:
                    {
                        result = writeSaveFile(JoInternalMemoryBackup, g_Game.saveName, saveFileData, saveFileSize);
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
                        result = writeSaveFile(JoCartridgeMemoryBackup, g_Game.saveName, saveFileData, saveFileSize);
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
                        result = writeSaveFile(JoExternalDeviceBackup, g_Game.saveName, saveFileData, saveFileSize);
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
                        result = writeSaveFile(SatiatorBackup, g_Game.saveFilename, saveFileData, saveFileSize);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_MODE:
                    {
                        jo_printf(OPTIONS_X, SAVES_Y + g_Game.numMenuOptions + 2, "Operation in progress....        ");
                        result = writeSaveFile(MODEBackup, g_Game.saveFilename, saveFileData, saveFileSize);
                        if (result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL;
                            return;
                        }
                        g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_WRITE_MEMORY:
                    {
                        transitionToState(STATE_WRITE_MEMORY);
                        //g_Game.operationStatus = OPERATION_SUCCESS;
                        return;
                    }
                    case SAVE_OPTION_DELETE:
                    {
                        // doesn't make sense to delete a memory dump
                        // BUGBUG: don't list this option for STATE_DISPLAY_MEMORY
                        if(g_Game.state == STATE_DISPLAY_MEMORY)
                        {
                            g_Game.operationStatus = OPERATION_FAIL_DELETE;
                            return;
                        }

                        result = deleteSaveFile(g_Game.backupDevice, g_Game.saveFilename);
                        if(result != 0)
                        {
                            g_Game.operationStatus = OPERATION_FAIL_DELETE;
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
            transitionToState(STATE_PREVIOUS);
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
    for(unsigned int i = 0; i < g_Game.numMenuOptions; i++)
    {
        jo_printf(OPTIONS_X, OPTIONS_Y + i, g_Game.menuOptions[i].optionText);
        y++; // spacing for the error messages
    }

    // cursor
    jo_printf(g_Game.cursorPosX, g_Game.cursorPosY + g_Game.cursorOffset, ">>");

    return;
}

// handles input on the format screen
void format_input(void)
{
    unsigned int option = 0;

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

            option = getMenuOptionByIndex(g_Game.cursorOffset);

            switch(option)
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

            transitionToState(STATE_PREVIOUS);
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
        transitionToState(STATE_PREVIOUS);
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
                        transitionToState(STATE_PREVIOUS);
                        return;
                    }

                    result = formatDevice(g_Game.backupDevice);
                    if(result != 0)
                    {
                        sgc_core_error("Failed to format device!!");
                        g_Game.operationStatus = OPERATION_FAIL;
                        transitionToState(STATE_PREVIOUS);
                        return;
                    }

                    g_Game.operationStatus = OPERATION_SUCCESS;
                    return;
                }
                case VERIFY_NO:
                {
                    transitionToState(STATE_PREVIOUS);
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

            transitionToState(STATE_PREVIOUS);
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

// draws the dump memory screen
void dumpMemory_draw(void)
{
    unsigned int y = 0;

    if(g_Game.state != STATE_DUMP_MEMORY && g_Game.state != STATE_WRITE_MEMORY)
    {
        return;
    }

    // heading
    if(g_Game.state == STATE_DUMP_MEMORY)
    {
        jo_printf(HEADING_X, HEADING_Y + y++, "Dump Memory");
    }
    else
    {
        jo_printf(HEADING_X, HEADING_Y + y++, "Write Memory");
    }

    jo_printf(HEADING_X, HEADING_Y + y++, HEADING_UNDERSCORE);

    y = 0;

    // options
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Address: 0x%08x", g_Game.dumpMemoryAddress);
    y++;
    jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Size:    0x%08x", g_Game.dumpMemorySize);
    y++;

    // digit cursor
    if(g_Game.cursorOffset < 8)
    {
        jo_printf(g_Game.cursorPosX + g_Game.cursorOffset, g_Game.cursorPosY, "^");
    }
    else
    {
        jo_printf(g_Game.cursorPosX + (g_Game.cursorOffset % 8), g_Game.cursorPosY + 2, "^");
    }

    jo_printf(HEADING_X, OPTIONS_Y + y++, "Press left/right to select digit");
    jo_printf(HEADING_X, OPTIONS_Y + y++, "Press up/down to adjust digit");
    jo_printf(HEADING_X, OPTIONS_Y + y++, "Address and size cannot be 0");
    jo_printf(HEADING_X, OPTIONS_Y + y++, "Press C to continue");
    y++;

    // done formatting
    if(g_Game.operationStatus == OPERATION_SUCCESS)
    {
        y++;
        jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Successfully performed operation.");
    }
    else if(g_Game.operationStatus == OPERATION_FAIL)
    {
        y++;
        jo_printf(OPTIONS_X, OPTIONS_Y + y++, "Failed to perform operation.     ");
    }

    y++;

    return;
}

// takes an integer and increments or decrements the specified hex digit
void adjustHexValue(unsigned int* value, unsigned int digit, bool add)
{
    unsigned int tempValue = *value;
    unsigned char nibbles[8] = {0};

    if(digit > 7)
    {
        sgc_core_error("Digit cannot be greater than 7!!");
        return;
    }

    // convert int to array of values
    for(int i = 0; i < 8; i++)
    {
        nibbles[7 - i] = tempValue & 0xF;
        tempValue = tempValue >> 4;
    }

    // add or subtract a value
    if(add == true)
    {
        nibbles[digit] = (nibbles[digit] + 1) & 0xF;
    }
    else
    {
        nibbles[digit] = (nibbles[digit] - 1) & 0xF;
    }

    // convert back to int
    tempValue = 0;
    for(int j = 0; j < 8; j++)
    {
        tempValue = tempValue << 4;
        tempValue += nibbles[j];
    }

    *value = tempValue;

    return;
}

// checks for up/down movement for the cursor
// erases the old one and draws a new one
// savesPage is set to true if the cursor is for the list saves page
void moveDigitCursor(void)
{
    int cursorOffset = g_Game.cursorOffset;
    int maxCursorOffset = g_Game.numStateOptions;

    // do nothing if there are no options
    // this can happen if we select a backup device with no saves for example
    if(g_Game.numStateOptions == 0)
    {
        return;
    }

    // left/right controls what digit we are looking at
    if (jo_is_pad1_key_pressed(JO_KEY_LEFT))
    {
        if(g_Game.input.pressedLeft == false)
        {
            cursorOffset--;
        }
        g_Game.input.pressedLeft = true;
    }
    else
    {
        g_Game.input.pressedLeft = false;
    }

    if (jo_is_pad1_key_pressed(JO_KEY_RIGHT))
    {
        if(g_Game.input.pressedRight == false)
        {
            cursorOffset++;
        }
        g_Game.input.pressedRight = true;
    }
    else
    {
        g_Game.input.pressedRight = false;
    }

    // up/down changes the digit value
    // left/right controls what digit we are looking at
    if (jo_is_pad1_key_pressed(JO_KEY_UP))
    {
        if(g_Game.input.pressedUp == false)
        {
            if(g_Game.cursorOffset < 8)
            {
                adjustHexValue(&g_Game.dumpMemoryAddress, g_Game.cursorOffset, true);
            }
            else
            {
                adjustHexValue(&g_Game.dumpMemorySize, g_Game.cursorOffset % 8, true);
            }
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
            if(g_Game.cursorOffset < 8)
            {
                adjustHexValue(&g_Game.dumpMemoryAddress, g_Game.cursorOffset, false);
            }
            else
            {
                adjustHexValue(&g_Game.dumpMemorySize, g_Game.cursorOffset % 8, false);
            }
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
        if(g_Game.cursorOffset < 8)
        {
            jo_printf(g_Game.cursorPosX + g_Game.cursorOffset, g_Game.cursorPosY, " ");
        }
        else
        {
            jo_printf(g_Game.cursorPosX + (g_Game.cursorOffset % 8), g_Game.cursorPosY + 2, " ");
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

// handles input on the dump memory and write memory screen
void dumpMemory_input(void)
{
    if(g_Game.state != STATE_DUMP_MEMORY && g_Game.state != STATE_WRITE_MEMORY)
    {
        return;
    }

    // did the player hit start
    if(jo_is_pad1_key_pressed(JO_KEY_START) ||
        jo_is_pad1_key_pressed(JO_KEY_A) ||
        jo_is_pad1_key_pressed(JO_KEY_C))
    {
        // it doesn't make sense to continue if the dump memory size or address
        // are zero so just ignore it and do nothing
        if(g_Game.input.pressedStartAC == false &&
            g_Game.dumpMemorySize != 0 &&
            g_Game.dumpMemoryAddress != 0)
        {
            g_Game.input.pressedStartAC = true;

            if(g_Game.state == STATE_DUMP_MEMORY)
            {
                // set globals based on the memory address and size being copied
                snprintf(g_Game.saveFilename, MAX_SAVE_FILENAME, "%08X.DM", g_Game.dumpMemoryAddress);
                g_Game.saveFileSize = g_Game.dumpMemorySize;

                transitionToState(STATE_DISPLAY_MEMORY);
                return;
            }
            else // STATE_WRITE_MEMORY
            {
                // check if we already did the write
                // if so, back output
                if(g_Game.operationStatus != OPERATION_UNINIT)
                {
                    transitionToState(STATE_PREVIOUS);
                    return;
                }

                // BUGBUG: print success messsage and then have some sort of wait here
                memcpy((void*)g_Game.dumpMemoryAddress, g_Game.saveFileData, g_Game.dumpMemorySize);
                g_Game.operationStatus = OPERATION_SUCCESS;
                return;
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

            transitionToState(STATE_PREVIOUS);
            return;
        }
    }
    else
    {
        g_Game.input.pressedB = false;
    }

    moveDigitCursor();
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

    jo_printf(OPTIONS_X - 3, OPTIONS_Y - 1 + y++, "Please submit your \".BUP\" files.");

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
            transitionToState(STATE_PREVIOUS);
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

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Johannes Fetz for Jo Engine and");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "adding features needed by SGC.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "EmeraldNova for doing all Satiator");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "testing.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Terraonion for contributing MODE");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "support.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "RevQuixo for numerous bug reports.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Antime, Ponut, VBT, and everyone");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "else at SegaXtreme keeping the");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Saturn dev scene alive.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "Thank you to Takashi for the");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "original Save Game Copier idea");
    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, "back in ~2002.");
    y++;

    jo_printf(OPTIONS_X - 3, OPTIONS_Y + y++, " - Slinga (github/slinga-homebrew)");

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
            transitionToState(STATE_PREVIOUS);
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

