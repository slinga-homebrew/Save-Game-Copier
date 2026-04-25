#include "vcd_card.h"

#define BIOS_IS_VCD_CARD_PRESENT (0x06000274) // contains a pointer to bios_is_mpeg_card_present routine
#define BIOS_GET_VCD_CARD_ROM (0x06000298) // contains a pointer to bios_is_mpeg_card_present routine

typedef int (*bios_is_vcd_card_present_fn)(int filtno);
typedef int (*bios_get_vcd_card_rom_fn)(unsigned int index, unsigned int size, void* dest);

// Check if the VCD card is present
bool vcdIsBackupDeviceAvailable(int backupDevice)
{
    bios_is_vcd_card_present_fn bios_is_vcd_card_present = NULL;
    int result = 0;

    if(backupDevice != VCDCardBackup)
    {
        return false;
    }

    bios_is_vcd_card_present = (bios_is_vcd_card_present_fn)(*(unsigned int*)BIOS_IS_VCD_CARD_PRESENT);
    if(!bios_is_vcd_card_present)
    {
        sgc_core_error("VCD: Failed to lookup bios func1\n");
        return false;
    }

    result = bios_is_vcd_card_present(0);
    if(result != 0)
    {
        sgc_core_error("VCD not present %x", result);
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

    bios_get_vcd_card_rom_fn bios_get_vcd_card_rom = NULL;
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

    bios_get_vcd_card_rom = (bios_get_vcd_card_rom_fn)(*(unsigned int*)BIOS_GET_VCD_CARD_ROM);
    if(!bios_get_vcd_card_rom)
    {
        sgc_core_error("VCD: Failed to lookup bios func2\n");
        return -3;
    }

    // TODO: what are these magic numbers
    result = bios_get_vcd_card_rom(246, 128, outBuffer);
    if(result < 0)
    {
        sgc_core_error("VCD: Read 1 error %x\n", result);
        return -4;
    }
    sgc_core_error("VCD: Read 1 Success %x\n", result);

    result = bios_get_vcd_card_rom(246 + 128, 128, outBuffer + 0x40000);
    if(result < 0)
    {
        sgc_core_error("VCD: Read 2 error %x\n", result);
        return -4;
    }
    sgc_core_error("VCD: Read 2 Success %x\n", result);

    return 0;
}
