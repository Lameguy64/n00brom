# n00bROM

n00bROM is an open source custom firmware for use in PS1 cheat devices or
generic homebrew cartridges that connect to the expansion port found in
SCPH-750x and older consoles. Functionally, n00bROM is similar to Caetla in
that it allows uploading and executing PS-EXEs on the console useful for
testing homebrew on real hardware.

Whilst n00bROM is fairly bare bones compared to Caetla and is more developer
oriented as it lacks cheat capabilities of any kind (and there are no plans to
implement such a capability), n00bROM makes up for it by featuring capabilities
not found in vanilla Caetla firmware; such as downloading PS-EXEs from serial,
serial TTY, supporting more EEPROMs, self-flash, nocash unlock and simplified
swap trick boot methods.

Whilst n00bROM may be used to boot backups, copies or whatever you want to call
it, this is NOT the primary purpose of n00bROM. n00bROM is primarily intended as
a tool for homebrew development and running homebrew on real hardware backed
by the fact it has mostly developer oriented features which are useless to
players and lacks any cheat functionality (and before you ask, this will not be
implemented). It is entirely up to the user how they use n00bROM as they see
fit and Lameguy64 is not responsible nor concerned of any liabilities it may
bring from 'improper' use of n00bROM if it ever does happen. Selling of cheat
devices with n00bROM pre-installed is highly discouraged.


## Features

* Automatic and manual video standard selection.

* Chipless CD-R boot capabilities; nocash unlock and simplified swap trick.
  It may not bypass region locked or libcrypt protected games however.

* Bypasses BIOS shell entirely, shortening boot times and bypasses pesky
  license data checks (such as PS logo data) found in some BIOS revisions,
  allowing to boot imports and eliminates the need of patching disc images.
  It can also boot completely unlicensed discs provided there is a SYSTEM.CNF
  or PSX.EXE file in the root of the CD-ROM file system.

* PS-EXE loader capability via serial or Xplorer parallel port interface. Uses
  mcomms/liteload style protocol for the serial loader.

* TTY redirection to serial or Xplorer parallel port for both stdin and stdout
  allowing for complete text console access for both input and output. Use
  mcomms in -term mode or any serial terminal program when using serial TTY.

* Access files from your PC through the PCDRV interface (Xplorer only).
  File create, read and write operations are implemented with parity checking
  and checksums for improved reliability. (use BIOS file functions, does not
  support PCDRV calls by break instructions)

* Exception handler for trapping software crashes visually.

* Supports more EEPROMs than Caetla with EEPROM and work-in-progress
  cartridge ID capability.

* Built-in EEPROM flasher for updating n00bROM easily.

* Animated plasma and SMPTE color bar backgrounds.

* Small ROM size; just under 32KB.

* Written entirely in MIPS assembly!


## Usage

n00bROM is fairly straight forward to use. Pressing START from the 'home' screen
will initiate the CD-ROM boot sequence and pressing SELECT will bring up the ROM
menu, where you can configure n00bROM to your preferences.

Whilst in the home screen, n00bROM will constantly listen for any PS-EXE or
binary uploads from the serial interface. If Cartridge Type is set to Xplorer in
the ROM menu, it will additionally listen for uploads from the parallel port
interface of the cartridge. However, it is not compatible with catflap so you'll
have to use the included xpcomms tool instead.

For serial uploads, you'll have to use mcomms to upload your PS-EXE to the serial
interface ( https://github.com/lameguy64/mcomms ). It behaves more or less like
having LITELOAD running on the target console.


Entering the ROM menu, the options available are as follows:

Video standard:
  
  Specifies the video standard n00bROM will use. Available options are Auto,
  NTSC and PAL. This option does not affect the video standard games will use.
  
  Auto works by checking the SCEx string within the BIOS ROM to identify the
  console's region. 

CD-ROM Boot Mode:
  
  Specifies the boot method that will be used as part of the CD-ROM boot
  sequence when pressing START from the 'home' screen or enabling Quick Boot.
  This option also takes effect when uploading a PS-EXE to the console.
  Available options are Normal, Unlock and EZ-Swap.
  
  Unlock allows regular CD-ROMs, CD-Rs and imports to become readable and
  bootable without the need of a mod-chip. This works by issuing 'secret'
  backdoor commands to the CD-ROM controller which enables the ability to read
  data sectors from unlicensed or out of region discs properly. This option only
  works for US, EU and Yaroze consoles.
  ( http://problemkaputt.de/psx-spx.htm#cdromsecretunlockcommands )
  
  EZ-Swap allows reading and booting CD-Rs by performing a swap trick which is
  made much easier with EZ-Swap, as it stops the disc during the swap procedure
  making it easy to perform without damaging anything from trying to swap the
  disc while spinning. However, just like the regular swap trick games or
  homebrew that use CD Audio will not work very well with this method as the
  TOC cannot be updated without loosing the 'authenticated' state of the CD-ROM
  controller. Obviously, an official copy of a game that works on your
  console's region is required for this procedure to work. This option is
  somewhat redundant as Unlock is the better overall option, but this is here
  for Japanese consoles that do not support Unlock.
    
TTY Interface:

  Specifies the interface to direct TTY terminal I/O (ie. printf() messages and
  getchar()/gets() input). Available options are Off, Serial and Xplorer.
  
  Serial should be self-explanatory. It directs TTY I/O to the Serial I/O port
  of the console at 115200 baud, 8 data bits, 1 stop bit, no parity and no
  handshaking. Pending input can be polled through ioctl(0, FIOCSCAN, 0) or
  ioctl(0, (('f'<<8)|2), 0).
  
  Xplorer directs TTY I/O to the parallel port interface of the cartridge.
  It is however currently not very reliable to get started when trying to use
  it with software booted from CD-ROM such as games.
  
PCDRV (Xplorer only):

  When enabled, n00bROM will install a special PCDRV device which can be used
  to access files directly from the host PC using BIOS standard file functions.
  This is akin to the pcdrv: device found in Caetla and official PS1
  development environments. PCDRV access by break opcodes (ie. libsn.h) are not
  supported.
  
  xpcomms must be running on the host PC as it works as the file server for
  PCDRV functionality.

Exception:

  When enabled, n00bROM will patch a custom exception handler to the kernel
  which will monitor for any unhandled exceptions or software crashes (ie. bad
  opcode, invalid/misaligned memory address break opcodes, etc) and displays a
  register dump at the point that caused the crash. Useful for tracing crashes
  without an interactive debugger.
  
Cartridge Type:

  Specifies the type of cartridge n00bROM is installed on. This is really only
  used for the switch detection which is handled differently between a PAR/GS
  and Xplorer cartridge. Available options are Generic (for carts without a
  switch, or a custom cart that is not supported yet), PAR/GS and Xplorer.
  Make sure you choose the correct option as otherwise you may render n00bROM
  unbootable on your cartridge.
  
  Xplorer will additionally enable listening of parallel port uploads from
  xpcomms.
  
OFF Switch Action:

  Specifies the action n00bROM will take if the switch on the cartridge is set
  to the OFF position. Available options are BIOS and Quick Boot. This option
  only works if Cartridge Type is set to PAR/GS or Xplorer.
  
  BIOS simply does not start n00bROM and the console will behave as if no
  cartridge was inserted.
  
  Quick boot simply skips the shell and goes straight to the boot sequence.
  The CD-ROM Boot Mode option still takes effect for this mode.
  
Background:

  Specifies the background image to display. Available options are Plasma and
  SMPTE color bars. The latter will feel right at home with the official
  development boards.

Home screen:

  Specifies the type of home screen to be displayed in the 'home' screen.
  Available options are Banner, Minimal and BG Only.
  
  Banner displays the large banner with a fairly long notice text.
  
  Minimal only displays a small box that shows some information about n00bROM
  and some of the current configuration such as TTY, exception and PCDRV.
  
  BG Only displays nothing but the background. This also disables the screen
  dimming which can be used in tandem with the SMPTE background for
  testing/calibrating video equipment.
  
Flash Mode:

  Enters flash mode for updating or replacing n00bROM. Can only flash up to the
  first 128K for PAR/GS and Xplorer cards and will not work if the EEPROM is
  ID'd as an unknown chip.

Save Settings:

  Saves whatever setting you have changed. This option will not do anything if
  the EEPROM is ID'd as an unknown chip.


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
( https://github.com/kingcom/armips ).

Building the ROM itself is just a matter of running build.sh under Linux with
a bash prompt or build.bat under Windows. It should produce romhead.bin,
ramprog.bin and n00brom.rom alongside the associated symbol/listing files.


## Credits

Main developer:
Lameguy64

Reference used:
nocash's PlayStation specs document ( http://problemkaputt.de/psx-spx.htm )

nicolasnoble's PS1 openbios reverse engineering project
( https://github.com/grumpycoders/pcsx-redux/tree/master/src/mips/openbios )
