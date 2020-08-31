# Save Game Copier (SGC)
Copy Sega Saturn save game files to and/or from internal memory, cartridge memory, external devices (e.g. Sega Saturn [Floppy Disk Drive](https://segaretro.org/Saturn_Floppy_Drive)), Satiator ODE, and CD. Build with [Jo Engine](https://github.com/johannes-fetz/joengine) or download an ISO from [releases](https://github.com/slinga-homebrew/Save-Game-Copier/releases). This is a very early alpha but seems to be useable. I need to do serious code cleanup\refactoring. 

## Screenshots
![Main](screenshots/main.png)
![List Saves](screenshots/saves.png)
![Copy](screenshots/copy.png)
![Satiator](screenshots/satiator.png)

## Satiator Support
I don't own a Satiator so my testing has solely been with the [Satiator Yabause fork](https://github.com/satiator/satiator-yabause). When using a Satiator:
* Make sure you upgrade to the latest firmware. There have been firmware fixes
* Create a "SAVES" directory on the root of the drive. SGC is hardcoded to use that folder

## Issues
* code cleanup\refactor
* program crashes if you run Satiator code without a Satiator. I don't know how to check for the presence of the Satiator safely yet.
* I want to make the menu dynamic based on what attachments are found (e.g. backup cartridge, Satiator, etc)
* Once you access the Satiator you can no longer list the saves in the "CD Memory" option. 

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

