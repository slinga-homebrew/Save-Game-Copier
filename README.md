# Save Game Copier (SGC)
Copy Sega Saturn save game files to and/or from internal memory, cartridge memory, external devices (e.g. Sega Saturn [Floppy Disk Drive](https://segaretro.org/Saturn_Floppy_Drive)), Satiator ODE, and CD. Build with [Jo Engine](https://github.com/johannes-fetz/joengine) or download an ISO from [releases](https://github.com/slinga-homebrew/Save-Game-Copier/releases). One of the most useful features of SGC is to create a custom SGC ISO with your own save game files and copy them to your Saturn. This is an alpha release but seems to be useable. 

SGC is for copying save games to a Saturn. To copy save games from Saturn -> PC use [Save Game Extractor](https://github.com/slinga-homebrew/Save-Game-Extractor).  

## Screenshots
![Main](screenshots/main.png)
![List Saves](screenshots/saves.png)
![Copy](screenshots/copy.png)
![Satiator](screenshots/satiator.png)

## Save Games Format
The save games are stored in a raw format (no header, no encoding, no metadata, etc). Many emulators add a header to the save. When working with a save from an emulator I recommend using [ss-save-parser](https://github.com/hitomi2500/ss-save-parser) and extracting as raw without any extra information. 

All saves on the Saturn MUST have 11 character names. All files on a CD are limited to 8 character names with 3 character extensions. If you want your ISO to work properly on a Saturn you have to rename the file to an 8.3 character file. For example if the file your adding is FPS_6MEN_01, rename to FPS_6MEN._01.

Additional examples:  
GRANDIA_001 -> GRANDIA_.001  
_DEATHTANK_ -> _DEATHTA.NK_  
THREE_DIRTY -> THREE_DI.RTY  
SGC automatically renames it back for you.

## Adding Custom Save Games
There are two ways to add your custom save game files to SGC:
1) (Windows) Using something WinISO add your save game file to the SAVES directory. Again read the instructions in "Save Game Format" so you have the correct type of file and filename. 
2) If you are comfortable compiling SGC, you can also add saves at build time. Checkout SGC from source. Add your save game files (in a raw format) to cd/SAVES/ and recompile. Again read the instructions in "Save Game Format" so you have the correct type of file and filename.  The newly built ISO will include your saves. 

## Satiator Support
I don't own a Satiator so my testing has solely been with the [Satiator Yabause fork](https://github.com/satiator/satiator-yabause). When using a Satiator:
* Make sure you upgrade to the latest firmware. There have been firmware fixes
* Create a "SAVES" directory on the root of the drive. SGC is hardcoded to use that folder

## Issues
* I want to make the menu dynamic based on what attachments are found (e.g. backup cartridge, Satiator, etc)
* Once you access the Satiator you can no longer list the saves in the "CD Memory" option. I don't know if this is an issue with Satiator or the Satiator-Yabause fork I am testing with. If you really want to transfer multiple "CD Memory" saves to your Satiator, transfer them to your Internal Memory first. From there you can transfer multiple saves to the Satiator without issue.
* Some Satiator users have reported the first transfer takes ~90 seconds and then all other transfers are fast. Professor Abrasive is aware of the issue. 

## Troubleshooting
When debugging saves that don't work, load SGC on both systems and verify the MD5 hash and file size of both saves. The most likely issue is that of metadata being included. The other common issue is the name of the file. 

## Saturn Save Games Collect Project
Want to share your save games on the web? Send them to the [Save Games Collect](https://ppcenter.webou.net/pskai/savedata/) project. Made by Cafe-Alpha, the author of the Gamer's Cartridge. Please append a ".RAW" to the save filename before submitting. 

## License
Licensed under GPL3 to comply with the Iapetus license. 

## 3rd Party Code
Save Game Copier uses code from:
* [Jo Engine](https://github.com/johannes-fetz/joengine) - MIT
* [MD5](http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5) - BSD
* [satiator-menu](https://github.com/satiator/satiator-menu) - MPL and GPL
* [iapetus](https://github.com/cyberwarriorx/iapetus) - GPL

## Credits
* Thank you to [Emerald Nova](https://github.com/EmeraldNova) for volunteering to test the code on his Satiator. 
* Special thanks to Antime, Ponut, VBT, and everyone else at SegaXtreme keeping the Saturn dev scene alive. 
* Thank you to Takashi for the original Save Game Copier idea back in ~2002. 
* [Shentokk](https://github.com/Shentokk) for information regarding emulator save game extraction

