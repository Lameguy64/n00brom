# n00bROM

n00bROM is an open source custom firmware for use in PS1 cheat devices or
generic homebrewn cartridges that connect to the expansion port found in
SCPH-750x and older consoles. Functionally, n00bROM is similar to Caetla in
that it allows uploading and executing PS-EXEs on the console useful for
testing homebrew on real hardware.

Whilst n00bROM is fairly bare bones compared to Caetla and is more developer
oriented as it lacks cheat capabilities of any kind (and there are no plans to
implement such a capability), n00bROM makes up for it by featuring capabilities
not found in vanilla Caetla firmware; such as downloading PS-EXEs from serial,
serial TTY, supporting more EEPROMs, self-flash, nocash unlock and simplified
swap trick boot methods.


## Features

* Automatic and manual video standard selection.

* Chipless CD-R boot capabilities; nocash unlock and simplified swap trick.

* Quick boot capability.

* Upload PS-EXEs and binary files over serial using mcomms/liteload protocol,
  or high speed upload over parallel port with an Xplorer using the included
  xpsend tool (Linux only, but should work with USB and PCIE parallel port
  adapters provided they support SPP).

* TTY over serial redirection for stdin and stdout, use mcomms in -term mode
  or any serial terminal program to interact (serial parameters are 115200 8n1).

* Exception screen for trapping software crashes visually.

* EEPROM and work-in-progress cartridge identification.

* Supports more EEPROMs than Caetla.

* Built-in flasher, for updating n00bROM easily!

* Background options: Animated plasma and SMPTE bars.

* Small ROM size; just under 32KB.

* Written entirely in MIPS assembly!


## Usage

Using n00bROM should be pretty self explanatory. Pressing start from the home
screen will initiate a CD-ROM boot sequence whilst pressing Select brings up
the ROM menu. From here, you can configure n00bROM to your liking.

* Video standard: Specifies the video standard the console will output at
  when n00bROM is running. Auto selects the standard based on the console's
  region string in the BIOS otherwise it initializes the video in the specified
  standard. This option does not force the video standard games would run at.

* CD-ROM Boot Mode: Specifies the boot method used for booting a CD-ROM or
  before a downloaded PS-EXE is executed. Normal boots the CD-ROM normally or
  no special operation is performed when a PS-EXE has been downloaded. Unlock
  performs the nocash unlock, allowing CD-Rs or imports to become readable on
  an unmodified console but it only works on US and EU region systems. Ez-Swap
  enables a swap trick procedure by simply stopping the disc and restarting
  it after pressing a button, making performing a swap trick much easier 
  without risk of damaging the disc or console. Ez-Swap, much like the original
  swap trick method, is only effective if you have a licensed PS1 CD-ROM with a
  matching region inserted prior to swapping it with your CD-R or imported disc.
  
* TTY over SIO: Specifies if a custom TTY hook which directs TTY messages
  through stdout to the serial port should be installed. The hook also directs
  incoming input from serial to stdin, so BIOS stdio input functions such as
  gets() and getchar() can be used. Calling ioctl(0, 0, 0) can be used to
  determine if there's pending input, it returns non-zero if there's input
  pending.

* Exception: Specifies if n00bROM's exception handler should be installed.
  The exception handler will trap any unhandled exceptions (ie. a software
  crash) and display a hexdump of all CPU registers which can be used to
  help determine the cause of the crash. The exception handler may break
  compatibility with some games, such as those that support lightgun
  controllers. This option must be disabled if using patches that installs
  a custom exception hook such as PSn00bDEBUG or when performing a EEPROM
  update.
  
* Cartridge Type: Specifies what type of cartridge n00bROM is installed on.
  Set the appropriate option in order for the OFF Switch Mode option to work
  properly and to enable high speed download over parallel port on an Xplorer
  cartridge. Choose Generic if you're using a homebrewn cartridge that n00bROM
  doesn't support.
  
* OFF Switch Mode: Specifies if setting the cartridge's switch to the OFF
  position would drop you to the BIOS or perform a quick boot. This option
  only works if you've set the Cartridge Type to PAR/GS or Xplorer.
  
* Background: Specifies what background image to display.

* Home screen: Specifies what type of home screen should be displayed when
  starting n00bROM. Banner displays the n00bROM banner and notice text, minimal
  only displays the n00bROM banner and a string of enabled options, BG Only
  only displays the background and disables the screen saver, useful when
  paired it with the SMPTE bars background for testing video equipment.

* Flash Mode: Enters flash mode for updating or replacing n00bROM. Can only
  flash up to the first 128K and will not work if the EEPROM is unknown.

Settings can be saved by selecting Save Settings in this menu. Saving only
works if the detected EEPROM is a known chip and will not do anything if the
EEPROM is unknown.

To upload a PS-EXE or binary file to the console, upload it to the console
via serial using mcomms ( https://github.com/lameguy64/mcomms ) while in the
idle screen (not in the ROM menu or when attempting to boot a disc). If using
an Xplorer cartridge and your PC has a parallel port, you can use the included
xpsend utility to upload PS-EXEs or binary files at high speed. When uploading
binary files, make sure you don't overwrite the last 64KB of RAM. Otherwise
you'll overwrite n00bROM and crash the console.


## Supported EEPROMs

The following lists EEPROMs n00bROM may support writing to when saving settings
or using the built-in flash capability. These chips are also the most commonly
used among various cheat devices, apart from the Cypress one.

Atmel:
  AT29C020
  AT29BV020
  AT29C040A
  AT29LV010A
  AT29xV040A
  
SST:
  29EE010
  29xE010
  29EE010A
  29xE010A
  29EE020
  29xE020
  29EE020A
  2xEE020A
  28SF040
  
Winbond:
  W29EE01x
  W29C020
  W29C040
  
Cypress:
  S29A1016J

Chips that n00bROM has been most tested with are SST 29EE020 and Winbond
W29C020. All other chips listed above are untested, but should work in theory.

More chips may be added by adding appropriate entries to the chipid_list in
tables.inc, but keep in mind that n00bROM only supports writing using the
JEDEC scheme with software data protection (issues a software data protect
enable command then immediately writes a page worth of bytes to the EEPROM).


## Building

Building n00bROM requires Kingcom's armips assembler
( https://github.com/kingcom/armips ). If you're running Windows,
you'll need MSys2 for the bash prompt needed to parse build.sh.

Building the ROM itself is just a matter of running build.sh. It should
produce a n00brom.rom file.


## Credits

Main developer:
Lameguy64

Reference used:
nocash's Playstation specs document (http://problemkaputt.de/psx-spx.htm)
