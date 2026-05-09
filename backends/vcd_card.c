#include <jo/vcd_card.h>
#include "vcd_card.h"


// Check if the VCD card is present
bool vcdIsBackupDeviceAvailable(int backupDevice)
{
    int result = 0;

    if(backupDevice != VCDCardBackup)
    {
        return false;
    }

    result = jo_vcd_card_is_present();
    if(result != 0)
    {
        //sgc_core_error("VCD not present %x", result);
        return false;
    }

    return true;
}

// List the firmware
int vcdListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
{
    unsigned int count = 0;

    if(backupDevice != VCDCardBackup)
    {
        return -1;
    }

    if(numSaves < 1)
    {
        // need at least one save available
        return -2;
    }

    strncpy(saves[0].filename, "VCD_CARD.BUP", MAX_FILENAME);
    strncpy(saves[0].name, "VCD_CARD", MAX_SAVE_FILENAME);
    strncpy(saves[0].comment, "FIRMWARE", MAX_SAVE_COMMENT);
    saves[0].language = 0, 
    saves[0].date = 0;
    saves[0].datasize = 512*1024;
    saves[0].blocksize = 0;

    count = 1;
    
    return count;
}

// Read the firmware
int vcdReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outSize)
{
    UNUSED_ARG(filename);
    
    int result = 0;

    if(backupDevice != VCDCardBackup)
    {
        return -1;
    }

    if(outBuffer == NULL || filename == NULL)
    {
        sgc_core_error("Save file data buffer is NULL!!");
        return -1;
    }

    if(outSize < VCD_CARD_FIRMWARE_SIZE)
    {
        sgc_core_error("VCD buffer too small");
        return -2;
    }

    // TODO: what are these magic numbers
    result = jo_vcd_card_get_vcd_card_rom(246, 128, outBuffer, outSize);
    if(result < 0)
    {
        sgc_core_error("VCD: Read 1 error %x\n", result);
        return -4;
    }
    sgc_core_error("VCD: Read 1 Success %x\n", result);

    result = jo_vcd_card_get_vcd_card_rom(246 + 128, 128, outBuffer + VCD_CARD_FIRMWARE_SIZE/2, outSize);
    if(result < 0)
    {
        sgc_core_error("VCD: Read 2 error %x\n", result);
        return -4;
    }
    sgc_core_error("VCD: Read 2 Success %x\n", result);

    return 0;
}
