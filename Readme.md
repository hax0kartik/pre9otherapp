# pre9otherapp
Otherapp payload which can be used on firmwares earlier than 9.2 to launch an arm9 payload from the sd card directly.

### Note
The otherapp payload which is produced by this repo can be used on all regions and 3ds consoles but work only between 4.X and 9.2

## Usage

The otherapp can be used with soundhax to launch an arm9 payload such as safeb9sinstaller

### Steps

 1. Get the soundhax file for your console and region
 2. Get the arm9 payload, safeb9sinstaller in this case
 3. Download the otherapp.bin from the releases page
 4. Put all the files on the root of your sd
 5. Launch soundhax

If everything goes correctly you will be in safeb9sinstaller, this might/might not work on the first try.

## Technical details

I have incorporated an edited version of svchax(memchunkhax) and brahma(firmlaunchhax) to gain arm11 and arm9 execution. So we first use memchunkhax to gain arm11 execution, then we map the arm11 and arm9 payload and then perform a firmlaunch.

## Building

You need to have devkitpro installed to build this. CTRULIB isn't required.

Simply open terminal and type 

> make

 to build this project.

## Credits

Huge parts of this project are parts of pre-existing projects. I would like to thank anyone who has aided in the development of [BrahmaLoader](https://github.com/d0k3/BrahmaLoader), [svchax](https://github.com/aliaspider/svchax), [ctrulib](https://github.com/smealum/ctrulib) and [ninjhax2.x](https://github.com/smealum/ninjhax2.x/tree/master/cn_secondary_payload)

I'll also like to thank my testers:- [@jason0597](https://github.com/jason0597), [@saibotu](https://github.com/saibotu), [@knight-ryu12](https://github.com/knight-ryu12) and [@frozenchen](https://github.com/frozenchen)

## Help

This hasn't been tested on all pre 9.2 firmwares yet so if could confirm it to be working that would be great!