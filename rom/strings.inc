;; Strings

notice:
.ascii	"This ROM firmware is free software.", 0x10
.ascii	"If you obtained n00bROM from a", 0x10
.ascii	"cartridge advertised as n00bROM", 0x10
.ascii	"pre-installed without permission", 0x10
.ascii	"from Lameguy64 himself, you're", 0x10
.ascii	"being ripped off and you must", 0x10
.ascii	"demand for a refund. Meido-Tek", 0x10
.ascii	"Productions is not responsible", 0x10
.ascii	"(nor concerned) of any liabilities", 0x10
.ascii	"that may occur for 'improper' use", 0x10 
.ascii	"of n00bROM and it's unlock/swap", 0x10
.ascii	"capabilities. It is entirely up to", 0x10
.ascii	"you how you wish to use n00bROM.", 0x10, 0x10
.ascii	"n00bROM and source code", 0x10
.ascii	"can be obtained from:", 0x10
.ascii	"github.com/lameguy64/n00brom", 0x10
.ascii	"lameguy64.net/?page=n00brom", 0x10, 0x00

text_sample:
.asciiz "STTY+EXCEPT+UNLK+XPDL+PCDRV"

text_option_stty:
.asciiz "STTY "
text_option_except:
.asciiz "EXCEPT "
text_option_unlock:
.asciiz "UNLK "
text_option_swap:
.asciiz "SWAP "
text_option_xpdl:
.asciiz "XPDL "

text_swap_start:
.asciiz	"Press START to EZ-swap boot CD..."

text_unlock_start:
.asciiz	"Press START to unlock boot CD..."

text_normal_start:
.asciiz	"Press START to boot CD..."

text_options:
.asciiz	"Press SELECT for ROM menu..."

text_init_cd:
.asciiz "Initializing CD-ROM..."

text_quick_boot:
.asciiz "Quick boot!"

text_unlock_cd:
.asciiz "Unlocking CD-ROM..."

text_no_disc:
.asciiz	"No or invalid disc inserted."

text_unlock_incompat:
.ascii	"This console does not support",0x10
.ascii	"CD-ROM unlock. Only US, EU and",0x10
.asciiz	"Yaroze systems support unlock."

text_swap:
.ascii	"Swap disc without",0x10
.asciiz	"releasing the lid switch."

text_continue:
.asciiz	"Press START to continue..."

text_retry:
.asciiz	"Press START to retry..."

text_return:
.asciiz	"Press CROSS to return..."

text_check_cd:
.asciiz	"Checking Disc..."

text_cnf_read:
.asciiz	"Reading SYSTEM.CNF..."

text_cnf_read_error:
.asciiz	"Error reading SYSTEM.CNF"

text_disc_ok:
.asciiz	"Disc OK!"

text_error_booting:
.asciiz	"Unable to load PS-EXE..."

text_booting:
.asciiz "Booting..."

text_download:
.asciiz "Downloading PS-EXE..."

text_download_bin:
.asciiz "Downloading data..."

text_download_pat:
.asciiz "Downloading patch..."

text_download_rom:
.asciiz "Downloading ROM..."

text_exec:
.asciiz "Execute!"

text_csum_fail:
.asciiz "Checksum Error"

text_rom_menu:
.asciiz "-- ROM Menu --"

text_flash_title:
.asciiz "-- Flash Mode --"

text_flash_info:
.ascii	"Upload ROM image with mcomms",0x10
.ascii	"using the binary upload option.",0x10
.ascii	"(ie. 'mcomms up n00brom.rom 0')",0x10
.ascii	"Address parameter is ignored.",0x10
.asciiz	"Only first 128KB can be flashed."

text_flashing:
.asciiz "Flashing..."

; Hex table for integer to hex string conversion
hex_tbl:
.ascii	"0123456789ABCDEF"
	
; For CNF parser and CD-ROM boot stuff
s_boot:
.asciiz "BOOT"
.align 4
s_tcb:
.asciiz "TCB"
.align 4
s_evcb:
.asciiz "EVENT"
.align 4
s_stack:
.asciiz "STACK"
.align 4
s_systemcnf:
.asciiz "cdrom:SYSTEM.CNF;1"
.align 4
s_psxexe:
.asciiz "cdrom:PSX.EXE;1"
.align 4

; HC05 region strings
s_europe_chk:
.asciiz "for Europe"
s_uscan_chk:
.asciiz "for U/C"
s_netna_chk:
.asciiz "for NETNA"
	
; Unlock strings
.ascii  "Not "
s_unlock_a:
.asciiz "Licensed by"
s_unlock_b:
.asciiz "Sony"
s_unlock_c:
.asciiz "Computer"
s_unlock_d:
.asciiz "Entertainment"
s_unlock_e_us:
.asciiz "of America"
s_unlock_e_eu:
.asciiz "(Europe)"
s_unlock_e_ww:
.asciiz "World wide"
.align 4


;; Menu stuff

list_menu_items:
dw	list_videomode
dh	0,3
dw	list_bootmode
dh	0,3
dw	list_ttymode
dh	0,2
dw	list_exception
dh	0,2
dw	list_cart_type
dh	0,3
dw	list_off_switch
dh	0,2
dw	list_background
dh	0,2
dw	list_home_screen
dh	1,3
dw	list_flash
dh	1,0
dw	list_save
dh	0,0
dw	list_exit
dh	0,0
dw	0
	
list_menu_options:
dw	list_videomode_options
dw	list_bootmode_options
dw	list_ttymode_options
dw	list_exception_options
dw	list_cart_options
dw	list_off_switch_options
dw	list_background_options
dw	list_home_options
dw	0
	
list_videomode_options:
dw	list_videomode_auto
dw	list_videomode_ntsc
dw	list_videomode_pal
	
list_bootmode_options:
dw	list_bootmode_normal
dw	list_bootmode_unlock
dw	list_bootmode_swap
	
list_ttymode_options:
dw	list_options_off
dw	list_options_on

list_cart_options:
dw	list_ctype_generic
dw	list_ctype_par
dw	list_ctype_xplorer
	
list_background_options:
dw	list_background_plasma
dw	list_background_bars
;dw	list_background_libbie
;dw	list_background_bunbun
	
list_home_options:
dw	list_home_banner
dw	list_home_simple
dw	list_home_bgonly
	
list_cdonexec_options:
dw	list_options_off
dw	list_options_on
	
list_exception_options:
dw	list_options_off
dw	list_options_on
	
list_off_switch_options:
dw	list_off_switch_bios
dw	list_off_switch_quickboot
;dw	list_off_switch_rom
	
list_options_on:
.asciiz "On"
list_options_off:
.asciiz "Off"
	
list_videomode:
.asciiz	"Video Standard"
list_bootmode:
.asciiz	"CD-ROM Boot Mode"
list_ttymode:
.asciiz "TTY over SIO"
list_cdonexec:
.asciiz	"Unlock/Swap on Exec"
list_background:
.asciiz "Background"
list_home_screen:
.asciiz "Home Screen",0x10
list_cart_type:
.asciiz "Cartridge Type"
list_off_switch:
.asciiz	"OFF Switch Mode"
list_exception:
.asciiz	"Exception"
list_flash:
.asciiz "Flash Mode...",0x10
list_flasher:
.asciiz "Start ROM Utility...",0x10
list_save:
.asciiz "Save Settings"
list_exit:
.asciiz "Exit ROM Menu"
	
list_videomode_auto:
.asciiz "Auto"
list_videomode_ntsc:
.asciiz "NTSC"
list_videomode_pal:
.asciiz "PAL(Eww)"
	
list_bootmode_normal:
.asciiz	"Normal"
list_bootmode_unlock:
.asciiz	"Unlock"
list_bootmode_swap:
.asciiz	"EZ-Swap"
	
list_ctype_generic:
.asciiz "Generic"
list_ctype_par:
.asciiz "PAR/GS"
list_ctype_xplorer:
.asciiz "Xplorer"

list_background_plasma:
.asciiz	"Plasma"
list_background_bars:
.asciiz	"SMPTE Bars"
;list_background_libbie:	; Might be added in the future
;.asciiz "Libbie"
;list_background_bunbun:
;.asciiz "Bun Bun Girl"
	
list_home_banner:
.asciiz "Banner"
list_home_simple:
.asciiz "Minimal"
list_home_bgonly:
.asciiz "BG Only"
	
list_off_switch_bios:
.asciiz "BIOS"
list_off_switch_quickboot:
.asciiz "Quick Boot"
;list_off_switch_rom:
;.asciiz "ROM Prog."
	
text_chip:
.asciiz "EEPROM: "
	
.align 4
	
; Cartridge strings
list_carts:
	dw	list_cart_par
	dw	list_cart_par_dual
	dw	list_cart_xplorer
	dw	list_cart_xplorerfx
	dw	list_cart_xplorerfx_dual
	dw	list_cart_xplorerfx_modded
	dw	list_cart_unknown

text_cart:
.asciiz "Cart: "

list_cart_par:
.asciiz "PAR/Gameshark"

list_cart_par_dual:
.asciiz "PAR/Gameshark (Dual EEPROM)"
			
list_cart_xplorer:
.asciiz "Xplorer"

list_cart_xplorerfx:
.asciiz "Xplorer FX"

list_cart_xplorerfx_dual:
.asciiz "Xplorer FX (Dual 256K)"

list_cart_xplorerfx_modded:
.asciiz "Xplorer FX (256K modded)"

list_cart_unknown:
.asciiz "Unknown cartridge"

; Loader related strings
str_mexe:
.asciiz "MEXE"
str_mbin:
.asciiz "MBIN"
str_mpat:
.asciiz "MPAT"

.align 4
