# Backends
Save Game Copier (SGC) currently supports the following backends to read\write save game files to:
* internal\cartridge\external (Floppy) (saturn.c)
* Action Replay Cartridge (actionreplay.c)
* CD (cd.c)
* MODE (mode.c)
* Satiator (satiator.c)

# Adding a New Backend
To be compatible with SGC, each new backend must support the following :
* safe detection of backend device
* change directory to /SATSAVES
* directory listing
* query file size
* read file
* write file (optional)
* delete file (optional)
* format device (optional)
* GPL3 or compatible license

## Backup.h
Edit backup.h adding a new #define for the new backup device. ActionReplayBackup is currently the last one.

## Backend Specific Source
Create a mybackend.h and mybackend.c. See saturn.h and saturn.c as examples. The file must have the following functions:

* bool mydeviceIsBackupDeviceAvailable(int backupDevice)
  * returns true if the backup device is present. Should be safe to call when the device isn't present.
* int mydeviceListSaveFiles(int backupDevice, PSAVES saves, unsigned int numSaves)
  * queries the saves on the backup device and fills out the fileSaves array. Returns the number of saves found.
* int mydeviceReadSaveFile(int backupDevice, char* filename, unsigned char* outBuffer, unsigned int outBufSize)
  * reads the save to outBuffer
* (optional) int mydeviceWriteSaveFile(int backupDevice, char* filename, unsigned char* saveData, unsigned int saveDataLen)
  * writes the save file
* (optional) int mydeviceDeleteSaveFile(int backupDevice, char* filename)
  * deletes the specified file

## Backend.c
Edit backend.c adding the new device to the switch statements for isBackupDeviceAvailalbe(), listSaveFiles(), readSaveFile(), writeSaveFile() (optional), deleteSaveFile() optional, and formatDevice() (optional).

Update getBackupDeviceName() to return the name of your device.

## Main.h
Add a #define MAIN_OPTION_MY_DEVICE. If your device is writeable, add a SAVE_OPTION_MY_DEVICE #define as well.

In the _GAME struct, add a bool deviceMyDeviceBackup member.

## Main.c
Add your device to queryBackupDevices(). g_Game.deviceMyDeviceBackup = isBackupDeviceAvailable(MyDeviceBackup).

Edit initMenuOptions() adding your device under STATE_MAIN, STATE_DISPLAY_SAVE (if your device is writeable), and STATE_FORMAT (if your device is formatable).

Edit main_input(), add a MAIN_OPTION_MY_DEVICE case.

Edit listSaves_draw() add a your device to the check.

Edit displaySave_input(), adding a case statement for your device.

## Makefile
Update the makefile
