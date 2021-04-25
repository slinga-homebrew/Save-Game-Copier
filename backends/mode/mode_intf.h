/*
* MODE SATURN BETA COMMAND INTERFACE.
* This library depends on the SEGA_CDC library
* If you use it in your own project and find a bug, or need the library source code, please contact us through discord
*/

struct _SatDirList
{
	char Name[124];
	Uint32 Size;
};

struct _SatGameList
{
	char Name[120];
	Uint32 NameChecksum;
	Uint8 Unused;
	Uint8 Flags;
	Uint16 GameID;
};


struct _MountStatus
{
	Uint8 SDStatus;
	Uint8 HDDStatus;
	Uint8 USBDetStatus;
	Uint8 Unused[5];
};

struct _VersionInfo
{
	Uint32 Serial;
	Uint8 Ver;
	Uint8 Subver;
	Uint8 Build;
	Uint8 dummy;
};

//Open and close the command stream. If you need to read from CDRom, you should close the command stream first
void MODE_Open();
void MODE_Close();

//Implement this function in your code with the wait for vsync function in the library you are using (slSynch() for SGL, SCL_DisplayFrame() for SBL)
extern void MODE_WaitVSync();

struct _MountStatus *MODE_GetMountStatus();
struct _VersionInfo* MODE_GetVersionInfo();




unsigned char MODE_OpenFile(const char* filename, unsigned char forwrite);
void MODE_DeleteFile(const char* filename);
void MODE_CloseFile();
void MODE_WriteFile(unsigned char* buffer, unsigned int offset, unsigned int size);
void MODE_ReadFile(unsigned char* buffer, unsigned int offset, unsigned int size);
Sint32 MODE_ReadFileListing(const char* path, struct _SatDirList* list, int maxfiles);
/*
* USING THE FILE I/O INTERFACE
* File IO is a simple interface to read/write data from SD and HDD. The basic sequence is:
* 
*	MODE_OpenFile()
*	Do some operations (MODE_WriteFile, MODE_ReadFile)
*	MODE_CloseFile()
* 
* Only one file can be open at a time. Also ensure to call MODE_CloseFile() for every MODE_OpenFile(), otherwise descriptors may leak internally.
* MODE_OpenFile returns 0 on success, any other value is an error.
* drive "0:/" is SD card, drive "1:/" is HDD. To check which drives are mounted, you can use MODE_GetMountStatus. 
*	If SDStatus or HDDStatus are 0, that means they are present and mounted. Other values mean error.
*	If USBDetStatus is not 13 (0xD) that means MODE is still scanning for SATA and USB disks, so don't attempt to access drive 1:/ , and also HDDStatus is not accurate.
* 
* There is no current directory in MODE, so all paths must be absolute!
* 
* MODE_ReadFileListing will list the files in the current directory. It will skip . and .. entries and any hidden file.
* Size is 0xFFFFFFFF for directories.
* Files are listed unsorted.
* The path must NOT end with a /, otherwise it will fail (0:/SATSAVES work but 0:/SATSAVES/ doesn't)
* 
* */


//Gamelist interface
void MODE_Initialize();
Sint32 MODE_ReadDirectoryListing(struct _SatGameList* gamelist);
int MODE_AddGame(Uint16 gameid, Uint8* ndiscs);
void MODE_ClearQueue();
int MODE_GetGameFlags(Uint8 disc);
void MODE_MountGame();
//Pass 0xFFFE or 0xFEFF to go to the UP directory.
int MODE_SelectDir(Uint16 gameid);
void MODE_HWReset();

/*
* USING THE GAMELIST INTERFACE (BETA)
* For the game list interface and launch to work properly, MODE_Initialize() must be called prior to any calls, as it setups the internal configuration and console region
* 
* Then, obtain the directory list with MODE_ReadDirectoryListing. Prepare a large enough structure to hold the data (3000 max) it returns the number of files in the list.
* If the listing belongs to a sub folder, then the first entry will always be "UP" with 0xFFFE game id.
* Returned list is not sorted.
* 
* Then, when selecting a game, call AddGame, passing the gameid from the gamelist struct. the resulting values can be:
*	0:			the game was a saturn game and has been queued (ndiscs receives the current disc count in the queue, after adding the game)
* -1 or 0xFF:	the selected item was a folder with more discs, so you can enter it by passing gameid to MODE_SelectDir(), then do MODE_ReadDirectoryListing again to get the games in the current directory.
* -2 or 0xFE:	the selected item is a disc, but it's not a saturn game (CDDA or other data cd), but has been queued anyways
* -3 or 0xFD:	the selected item is not a disc.
* 
* If you want to launch the current queue (right after adding a game, or when the users selects to launch), first you must call MODE_GetGameFlags() with the disc number to load (0 to 7). This will
* parse the cue file and preload the system sectors.
* Once you call this function, you won't be able to read the CD anymore, so ensure all your assets are in ram.
* The returned value is the value set in Flags in MODE.CFG file in the disc folder.
* 
* Then call MODE_MountGame(). From now on, the game disc is "inserted", and the CD Block will have read its TOC.
* 
* Then you must load the game into memory (fastload) or fully reset the system. You can just call MODE_HwReset() to fully reset the system via SMPC.
* 
*/

