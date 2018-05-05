# pre9otherapp
Otherapp payload which can be used on firmwares earlier than 4.0 to launch an arm9 payload from the sd card directly.

### Note
The otherapp payload which is produced by this repo can be used on all regions and 3ds consoles but work only between 1.X and 4.X

## Usage

The otherapp can be used with soundhax to launch an arm9 payload such as safeb9sinstaller.
*The otherapp looks for a arm9.bin on the root of your sd card.*

### Steps

 1. Get the soundhax file for your console and region
 2. Get the arm9 payload, safeb9sinstaller in this case
 3. Download the otherapp.bin from the releases page
 4. Put all the files on the root of your sd(for safeb9sinstaller, just copy the arm9.bin from the zip)
 5. Launch soundhax

If everything goes correctly you will be in safeb9sinstaller, this might/might not work on the first try.

## Technical details

I have incorporated an edited version of usr2arm9ldr which launches the arm9 payload. [usr2arm9ldr](https://github.com/TuxSH/usr2arm9ldr) was written by TuxSH.

## Building

You need to have devkitpro installed to build this. CTRULIB isn't required.

Simply open terminal and type 

> make

 to build this project.

## Credits

Huge parts of this project are parts of pre-existing projects. I would like to thank anyone who has aided in the development of [Usr2arm9ldr](https://github.com/TuxSH/usr2arm9ldr) and [ctrulib](https://github.com/smealum/ctrulib)

Massive thanks to my testers:- 

[@jason0597](https://github.com/jason0597)
[@ihaveamac](https://github.com/ihaveamac)

Wouldn't have been able to do it without you guys!

Also thanks to TuxSH for helping me out with various questions!