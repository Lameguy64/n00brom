# n00bROM

n00bROM is an open source custom firmware for use in PS1 cheat devices or
generic homebrew cartridges that connect to the expansion port found in
SCPH-750x and older models. Functionality wise, n00bROM is similar to Caetla
in that it allows uploading and executing PS-EXEs on the console, ideal for
testing PS1 programs on real hardware in homebrew development or ROM hacking.

Whilst n00bROM is fairly bare bones compared to Caetla and is more developer
oriented, n00bROM makes up for it by featuring capabilities not found in
vanilla Caetla; such as PS-EXE download via serial, serial TTY, supporting
more EEPROMs, built-in flash option for quick updating, nocash unlock and
simplified swap trick boot options.

While n00bROM may be used to boot backups, copies or whatever you want to
call it, this is NOT the main  purpose of n00bROM. n00bROM is primarily
intended as a tool for homebrew development and getting homebrew to run on
real hardware, supported by the fact it has mostly developer oriented
features and lacks any sort of cheat functionality (and before you ask, this
will not be implemented). It is entirely up to the user how they use n00bROM
as they see fit, Lameguy64 is not responsible nor concerned of any
liabilities it may bring from the use of n00bROM, if such accusations ever
do happen. If you don't feel comfortable using n00bROM, do not use n00bROM.

Selling of cheat devices with n00bROM pre-flashed is highly discouraged
however.


## Features

* Automatic and manual video standard selection.

* Chipless CD-R boot capabilities; nocash unlock and simplified swap trick.
  It may not bypass region locked (internal lock, not by BIOS or CD-ROM) or
  libcrypt protected games however.

* Bypasses BIOS shell entirely, shortening boot times and bypasses pesky
  license data checks (such as PS logo data) found in some BIOS revisions,
  allowing to boot imports and eliminates the need to patch disc images.

* Can boot completely unlicensed discs provided there is a SYSTEM.CNF or
  PSX.EXE file is present in the root.

* PS-EXE loader capability via serial or Xplorer parallel port interface.
  Uses mcomms/liteload style protocol for the serial loader.

* TTY redirection via serial or Xplorer parallel port for both stdin and
  stdout text streams. Use mcomms in -term mode or any serial terminal
  program when using serial TTY.

* Access files from host system via the PCDRV interface (Xplorer only).
  Create, read and write operations are implemented with parity checking
  and checksums for improved reliability. Directory query and changing
  current directories are also supported (use BIOS file functions, does
  not support libsn style PCDRV calls by special break opcodes)

* Exception handler for visually trapping software crashes.

* Supports more EEPROMs than Caetla with EEPROM ID and work-in-progress
  cartridge ID capabilities.

* Built-in EEPROM flasher for updating n00bROM easily.

* Animated plasma and SMPTE color bar backgrounds.

* Small ROM size; just under 32KB.

* Written entirely in MIPS assembly!


## Usage

n00bROM is fairly straight forward to use. Pressing START from the 'home'
screen will initiate the CD-ROM boot sequence and pressing SELECT will bring
up the ROM menu, where you can configure n00bROM to your liking.

Whilst in the home screen, n00bROM will constantly listen for any PS-EXE or
binary uploads from the serial interface. If Cartridge Type is set to Xplorer
in the ROM menu, it will additionally listen for uploads from the parallel
port interface on the cartridge. However, it is not compatible with catflap,
so you'll have to use the included xpcomms tool instead.

For serial uploads, you'll have to use mcomms to upload your PS-EXE to the
serial interface ( https://github.com/lameguy64/mcomms ). It behaves more or
less like running LITELOAD on the target console.


Entering the ROM menu, the options available are as follows:

Video standard:
  
  Specifies the video standard n00bROM will use. Available options are Auto,
  NTSC and PAL. This option does not affect the video standard games will
  use.
  
  Auto works by checking the SCEx string within the BIOS ROM to identify the
  system's region. 

CD-ROM Boot Mode:
  
  Specifies the boot method that will be used as part of the CD-ROM boot
  sequence, when pressing START from the 'home' screen or enabling Quick Boot.
  This option also takes effect when uploading a PS-EXE to the console, in
  between download completion and transferring execution to the PS-EXE.
  Available options are Normal, Unlock and EZ-Swap.
  
  Unlock allows regular CD-ROMs, CD-Rs and imports to become readable and
  bootable without a mod-chip. This works by issuing 'secret' backdoor
  commands to the CD-ROM controller which enables reading of data sectors
  from unlicensed or out of region discs. This option only works for US, EU
  and Yaroze consoles.
  ( http://problemkaputt.de/psx-spx.htm#cdromsecretunlockcommands )
  
  EZ-Swap allows reading and booting CD-Rs by allowing a swap trick to be
  performed easily, as it stops the disc during the swap procedure to allow
  for swapping without the risk of damaging the disc or console. As of 0.28b,
  games and homebrew that use CD Audio will work properly by issuing a
  SetSession command to reload the ToC without clearing the authentication
  state, though accessing newer sessions in a multi-session disc is not
  possible. Obviously, an official copy of a game that matches the console's
  region is required for this procedure. This option may seem redundant as
  Uunlock is the better overall option, but this is here for Japan region
  consoles which do not support Unlock.
  
TTY Interface:

  Specifies which interface to direct TTY access (ie. printf() output and
  getchar()/gets() input). Available options are Off, Serial and Xplorer.
  
  Serial directs TTY to the Serial I/O port of the console at 115200 baud,
  8 data bits, 1 stop bit, no parity and no handshaking. Pending input can
  be polled through ioctl(0, FIOCSCAN, 0), or ioctl(0, (('f'<<8)|2), 0) if
  FIOCSCAN is not available.
  
  Xplorer directs TTY to the parallel port interface on the Xplorer
  cartridge. It is currently not very reliable to get working with software
  booted from CD-ROM, such as games however.
  
PCDRV (Xplorer only):

  When enabled, n00bROM will install a special PCDRV device into the BIOS
  kernel, which can be used to access files directly from the host system
  using standard BIOS file functions. No special library or headers required
  to take advantage of this feature.
  
  This is akin to the pcdrv: device found in Caetla and official PS1
  development environments. PCDRV access by break opcodes (ie. libsn.h) are
  not supported however.
  
  xpcomms must be running on the host PC, to serve as the file server for
  PCDRV functionality.

Exception:

  When enabled, n00bROM will patch a hook to the BIOS function vector,
  trapping unhandled exceptions (SystemErrorUnresolvedException - A(40h))
  that displays a killscreen showing all relevant processor registers and
  stack values of when the unhandled exception occured (ie. reserved opcodes,
  invalid/misaligned memory address break opcodes, etc), useful for
  identifying software crashes without an interactive debugger.
  
  This option may be removed in future versions of n00bROM, as the new BIOS
  function vector method is less likely to introduce compatibility issues,
  unlike the old method which patches a hook to the exception vector which
  can introduce compatibility problems.
  
  n00bROM also hooks a killscreen routine for boot failures
  (SystemErrorBootOrDiskFailure - A(A1h)) which cannot be disabled, a
  leftover from testing the new BIOS based bootstrap method. This is still
  useful for discriminating from a software crash after booting, or a disc
  error during booting.
  
Cartridge Type:

  Specifies the type of cartridge n00bROM has been flashed on. This is only
  really used for which switch detection logic to use, which is handled
  differently between PAR/GS and Xplorer cartridges. Available options are
  Generic (for carts without a switch, or a custom cart that is not supported
  yet), PAR/GS and Xplorer. Make sure you choose the correct option as
  otherwise you may render n00bROM unbootable on your cartridge by setting
  the wrong cartriddge type.
  
  Setting Xplorer also enables PS-EXE and binary downloads from the parallel
  port interface, using the included xpcomms tool.
  
OFF Switch Action:

  Specifies what action n00bROM will take if the switch on the cartridge has
  been set to the OFF position. Available options are BIOS and Quick Boot.
  This option only works if Cartridge Type is set to PAR/GS or Xplorer.
  
  BIOS would simply skip starting n00bROM, and the console will behave as if
  no cartridge was inserted. All n00bROM features are also disabled.
  
  Quick boot simply bypasses the shell and jumps straight to the bootstrap
  sequence. TTY, Exception and PCDRV hooks are also set when enabled. The
  CD-ROM Boot Mode option also takes effect for this mode.
  
Background:

  Specifies the background image to display. Available options are Plasma and
  SMPTE color bars. The latter will feel right at home with the official
  development boards.
  
  More backgrounds may be added in the future, if ROM space allows for such
  bells and whistles to be implemented.

Home screen:

  Specifies the type of home screen to be displayed in the 'home' screen,
  shown when starting n00bROM. Available options are Banner, Minimal and
  BG Only.
  
  Banner displays the large banner with a fairly long notice text.
  
  Minimal only displays a small box that shows some information about n00bROM
  and enabled configuration settings such as STTY (serial TTY),
  XTTY (Xplorer TTY), XPDL (Xplorer Download), Exception and PCDRV.
  
  BG Only, as the name suggests, only displays the background. This also
  disables the screen dimming, so it can be used in combination with the
  SMPTE background option for testing/calibrating video equipment.
  
Flash Mode:

  Enters Flash Mode for updating or replacing n00bROM. n00bROM can only flash
  the first 128K currently. The Flash Mode option will not work if the EEPROM
  is ID'd as an unknown chip.

Save Settings:

  Saves configuration setting you have changed to the EEPROM. This option will
  not work if the EEPROM is ID'd as an unknown chip.


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
standard JEDEC method, with software data protection commands (issues a
software data protect enable command then immediately writes a page worth of
bytes to the EEPROM).


## Building

Building n00bROM requires Kingcom's armips assembler
( https://github.com/kingcom/armips ).

Building the ROM itself is just a matter of running build.sh under Linux with
a bash prompt or build.bat under Windows. It should produce romhead.bin,
ramprog.bin and n00brom.rom alongside the associated symbol/listing files.


## Installing

Currently, the only feasible way to flash a ROM to a cheat cartridge is by
either desoldering the EEPROM and flashing the chip with an EEPROM
programmer, or using a really old tool called X-Flash, which requires manual
preparation of the disc in a specific manner and requires a chipped console
to actually get it to boot.

To help remedy this problem, a new flash utility made using PSn00bSDK is in
the works, and will support downloading ROM images from serial and reading
multi-session discs, so CD-Rs will be wasted less and to make installing
new versions of n00bROM easier.


## Credits

Main developer:
Lameguy64

Reference used:
nocash's PlayStation specs document ( http://problemkaputt.de/psx-spx.htm )

nicolasnoble's PS1 openbios reverse engineering project:
( https://github.com/grumpycoders/pcsx-redux/tree/master/src/mips/openbios )
