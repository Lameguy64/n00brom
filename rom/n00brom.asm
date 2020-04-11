.psx

.include "cop0regs.inc"
.include "hwregs.inc"

PROG_addr	equ		0x801EFFF0
SP_base		equ		0x801ffff0


INT_pc		equ		0
INT_sp		equ		4
INT_fp		equ		8
INT_s0		equ		12
INT_s1		equ		16
INT_s2		equ		20
INT_s3		equ		24
INT_s4		equ		28
INT_s5		equ		32
INT_s6		equ		36
INT_s7		equ		40
INT_gp		equ		44
INT_size	equ		48


VAR_vsync	equ		0
VAR_clut	equ		4
VAR_db		equ		8
VAR_ot		equ		12
VAR_paddr	equ		16
VAR_cdcmd	equ		20		; CD sync stuff
VAR_cdiwait	equ		21
VAR_cdinum	equ		22
VAR_cdival	equ		23
VAR_cdresp	equ		24		; CD response buffer
VAR_psexe	equ		40
VAR_padbuff	equ		72
VAR_siobuff	equ		140
VAR_sioaddr	equ		148
VAR_sioread	equ		152
VAR_xpready	equ		156
VAR_pbuff	equ		160


SET_vmode		equ		0		; Video mode
SET_bmode		equ		1		; CD-ROM boot mode
SET_tty			equ		2		; TTY enable
SET_except		equ		3		; Exception handler
SET_carttype	equ		4		; Cartridge type
SET_offact		equ		5		; Off switch action
SET_bg			equ		6		; Background
SET_homemode	equ		7		; Home screen mode


BRK_vector	equ		0xa0000040
BRK_addr	equ		0x80030000
prog_dest	equ		0x801EFFF0


.macro rfe
	.word 0x42000010
.endmacro

.macro EnterCriticalSection
	addiu	a0, r0, 0x1
	syscall	0
.endmacro

.macro ExitCriticalSection
	addiu	a0, r0, 0x2
	syscall	0
.endmacro


;; RAM resident program
.create "ramprog.bin", PROG_addr-8

	dw		PROG_addr				; Simple header for copy routine in ROM
	dw		(program_end-program_start)
	
program_start:

	la		sp, SP_base
	
	addiu	sp, -INT_size		; INT hook entry structure
	move	s0, sp
	addiu	s1, sp, -4			; INT hook stack
	addiu	sp, -1024
	
	la		gp, gp_start		; GP memory area (end of RAM code)
	
	mtc0	r0, DCIC			; Clear debug control register - not needed
	
	la		a0, int_handler		; Create HookEntryInt hook structure
	sw		a0, INT_pc(s0)
	sw		s1, INT_sp(s0)
	sw		r0, INT_fp(s0)
	sw		r0, INT_s0(s0)
	sw		r0, INT_s1(s0)
	sw		r0, INT_s2(s0)
	sw		r0, INT_s3(s0)
	sw		r0, INT_s4(s0)
	sw		r0, INT_s5(s0)
	sw		r0, INT_s6(s0)
	lui		v0, 0x1F80
	sw		v0, INT_s7(s0)
	sw		gp, INT_gp(s0)
	
	move	a0, s0				; Hook custom IRQ handler
	jal		HookEntryInt
	addiu	sp, -4
	addiu	sp, 4
	
	move	a0, r0				; Disable VSync IRQ auto acknowledge
	jal		ChangeClearPAD
	addiu	sp, -4
	addiu	sp, 4
	
	li		a0, 3				; Disable root counter IRQ auto ack
	move	a1, r0
	jal		ChangeClearRCnt
	addiu	sp, -8
	addiu	sp, 8
	
	lui		a0, 0x1F80			; Enable VSync IRQ
	li		v0, 0x5
	sw		v0, IMASK(a0)
	
	sw		r0, VAR_vsync(gp)	; Clear a number of global variables
	sw		r0, VAR_paddr(gp)
	sb		r0, VAR_db(gp)
	sw		r0, VAR_cdcmd(gp)
	
	sw		r0, VAR_sioaddr(gp)	; Clear SIO buffers
	sw		r0, VAR_sioread(gp)
	
	lui		v0, 0x3b33			; Enables DMA channel 6 (for ClearOTag)
	ori		v0, 0x3b33			; Enables DMA channel 2 (for GPU)
	sw		v0, DPCR(a0)
	sw		r0, DICR(a0)		; Clear DICR (not needed)
	
	la		v1, config
	lbu		v1, SET_except(v1)
	
	la		a0, install_exception
	beqz	v1, @@no_exception	; Don't install if not enabled
	nop
	
	jalr	a0					; Install exception handler
	nop
	
@@no_exception:

	la		v0, config
	lbu		v0, SET_carttype(v0)	; Skip Xplorer stuff if not set for Xplorer
	nop
	bne		v0, 2, @@skip_xplorer
	nop
	
	; Xplorer stuff
	lui		a0, XP_IOBASE		; Keep last PC handshake state
	lbu		v0, XP_pcshake(a0)
	sb		r0, XP_latch(a0)	; Clear latch
	andi	v0, 0x1
	sb		v0, VAR_xpready(gp)

@@skip_xplorer:

	j		main				; Jump to main
	rfe
	nop


init:

	addiu	sp, -4
	sw		ra, 0(sp);
	
	jal		InitGPU					; Init GPU
	nop
	
	la		a0, font_tim			; Load font TIM
	jal		LoadTIM
	addiu	a1, gp, VAR_clut
	
	lw		v0, VAR_clut(gp)
	nop
	srl		v1, v0, 16
	andi	v0, 0xFFFF				; Calculate CLUT X value
	srl		v0, 4
	andi	v1, 0x1FF
	sll		v1, 6
	or		v0, v1
	sh		v0, VAR_clut(gp)
	
	jal		cd_setup				; Setup CD-ROM I/O ports
	nop
	
	addiu	v0, r0, -1				; Clear pad buffer
	sw		v0, VAR_padbuff(gp)
	sw		v0, VAR_padbuff+4(gp)
	sw		v0, VAR_padbuff+8(gp)
	sw		v0, VAR_padbuff+12(gp)
	
	jal		PadInit					; Initialize pad interface
	nop
	
	jal		sioInit					; Initialize serial
	nop

	la		a0, config				; Install n00bROM TTY device if enabled
	lbu		v0, SET_tty(a0)
	nop
	beqz	v0, @@no_tty
	nop
	
	la		a0, InstallTTY
	jalr	a0
	nop
	
@@no_tty:

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
		

main:

	jal		init					; do init
	nop
	
	la		a0, config				; Get switch setting
	lbu		a0, SET_carttype(a0)
	nop
	beqz	a0, back_to_menu
	nop
	beq		a0, 1, @@switch_par
	nop
	
	lui		a0, 0x1F06				; Xplorer switch detection
	lbu		a0, 0(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@do_quickboot
	nop
	
	b		back_to_menu
	nop
	
@@switch_par:						; PAR/GS switch detection

	lui		a0, 0x1F02
	lbu		a0, 0x18(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@do_quickboot
	nop
	
@@do_quickboot:
	
	la		v0, config
	lbu		v0, SET_offact(v0)
	nop
	
	
	move	s0, r0
@@wait:
	la		a0, text_quick_boot
	jal		sort_status
	nop
	blt		s0, 60, @@wait
	addiu	s0, 1
	
	b		boot
	nop
	
back_to_menu:

	jal		idle_screen
	nop

	move	a0, r0					; Stop SIO receive
	jal		sioSetRead
	move	a1, r0

	beqz	v0, boot
	nop
	
	jal		rom_menu
	nop
	
	beqz	v0, back_to_menu
	nop
	
	beq		v0, 2, flash_mode
	nop
	
	EnterCriticalSection			; Reset
	
	lui		a0, 0xBFC0
	jr		a0
	nop
	

boot:								; Boot routine

	la		v0, config
	lbu		v0, SET_bmode(v0)
	nop
	beq		v0, 1, @@unlock_boot
	nop
	beq		v0, 2, @@swap_boot
	nop
	
	jal		init_disc
	nop
	j		boot_disc
	nop

@@unlock_boot:
	jal		init_disc
	nop
	jal		unlock_cdrom
	nop
	beqz	v0, back_to_menu
	nop
	j		boot_disc
	nop
	
@@swap_boot:
	jal		init_disc
	nop
	jal		swap_trick
	nop
	j		boot_disc
	nop


init_disc:							;; Low-level CD init

	addiu	sp, -4
	sw		ra, 0(sp)
	
	jal		cd_init					; Init CD (low level)
	nop
	
@@init_loop:

	la		a0, text_init_cd
	jal		sort_status
	nop

	lbu		v0, VAR_cdinum(gp)		; Wait until two CD IRQs occur on init
	nop
	blt		v0, 2, @@init_loop
	nop

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
flash_mode:							; Flash mode routine
	
	addiu	a0, gp, VAR_siobuff
	jal		sioSetRead
	addiu	a1, r0, 4

@@download_wait:

	jal		prep_frame
	nop
	
	la		a0, text_flash_title
	jal		DrawText_centered
	li		a1, 0x5c
	
	la		a0, text_flash_info
	jal		DrawText_multiline
	li		a1, 0x6c
	
	la		a0, 0x00500018
	la		a1, 0x00500110
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
	addiu	a0, gp, VAR_siobuff		; SIO binary loader
	la		a1, str_mbin
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, VAR_sioread(gp)
	beqz	v0, @@proceed_download
	nop
	
	lw		v0, VAR_sioread(gp)		; Retry
	nop
	beqz	v0, flash_mode
	nop
	
	b		@@download_wait
	nop
	
@@proceed_download:

	jal		sio_binloader			; Download the ROM image
	addiu	a0, r0, 2
	beqz	v0, flash_mode			; Retry download if checksum failed
	nop
	
	move	s0, v0					; Copy image size to s0
	
	addiu	s0, 255					; Round it to multiples of 256
	srl		s0, 8
	sll		s0, 8

	ble		s0, 131072, @@no_size_cap	; Cap size to 128KB	just in case
	nop
	li		s0, 131072
@@no_size_cap:

	jal		ChipIdentify			; Get chip page size
	move	a0, r0
	jal		ChipPageSize
	move	a0, v0
	move	s1, v0
	
	jal		prep_frame				; Display flashing message
	nop
	la		a0, text_flashing
	jal		DrawText_centered
	li		a1, 0x74
	jal		frame_done
	addiu	a2, r0, 1
	
	lui		t0, 0x8001				; ROM source address
	move	t1, r0
@@flash_loop:
	move	a0, t0
	move	a1, t1
	jal		ChipWrite
	move	a2, s1
	addu	t1, s1
	blt		t1, s0, @@flash_loop
	addu	t0, s1
	
	lui		a0, 0x8001				; Do verification
	lui		a1, 0x1F00
	move	a2, s0
@@verify_loop:
	lbu		v0, 0(a0)
	lbu		v1, 0(a1)
	addiu	a0, 1
	bne		v0, v1, verify_error
	addiu	a2, -1
	bgtz	a2, @@verify_loop
	addiu	a1, 1
	
	jal		prep_frame				; Display flash OK message
	nop
	la		a0, text_flash_ok
	jal		DrawText_centered
	li		a1, 0x74
	jal		frame_done
	addiu	a2, r0, 1
	
@@infini_loop:
	b		@@infini_loop
	nop
	
verify_error:
	jal		prep_frame
	nop
	la		a0, text_flash_verify_error
	jal		DrawText_centered
	li		a1, 0x74
	jal		frame_done
	addiu	a2, r0, 1
@@infini_loop:
	b		@@infini_loop
	nop
	
text_flash_verify_error:
.asciiz "Verification error."
text_flash_ok:
.asciiz "Flash OK!"
.align 0x4
			
	
idle_screen:

	addiu	sp, -4
	sw		ra, 0(sp)

	addiu	sp, -32
	move	s7, sp
	
	lw		s0, VAR_vsync(gp)
	nop
	srl		s0, 2
	
	sw		r0, VAR_siobuff(gp)			; Clear SIO temp buffer
	sw		r0, VAR_siobuff+4(gp)
	
	addiu	a0, gp, VAR_siobuff
	jal		sioSetRead
	addiu	a1, r0, 4
	
@@loop:

	jal		prep_frame
	nop

	la		v0, config
	lbu		v0, SET_homemode(v0)
	nop
	beq		v0, 2, @@skip_screensave
	nop
		
	lw		v0, VAR_vsync(gp)			; Screen saver that activates in 2 minutes
	nop	
	srl		v0, 2
	subu	v0, s0
	ble		v0, 1800, @@skip_screensave
	nop
	
	la		a0, 0x00000000
	la		a1, 0x01000140
	jal		sort_rect
	move	a2, r0
	
@@skip_screensave:
	
	la		v0, config
	lbu		v0, SET_homemode(v0)
	nop
	
	bnez	v0, @@skip_banner
	nop
	
	la		a0, text
	jal		DrawText_centered
	li		a1, 0x18
	
	la		a0, copyright
	jal		DrawText_centered
	li		a1, 0x20
	
	la		a0, notice
	jal		DrawText_multiline
	li		a1, 0x30
	
	
	li		a0, 24
	li		a1, 40
	jal		DrawText
	addiu	a2, gp, VAR_siobuff
	
	
	la		a0, config
	lbu		v0, 0(a0)
	nop
	beq		v0, 1, @@text_unlock
	nop
	beq		v0, 2, @@text_swap
	nop
	
	la		a0, text_normal_start
	b		@@text_draw
	nop
	
@@text_unlock:

	la		a0, text_unlock_start
	b		@@text_draw
	nop
	
@@text_swap:

	la		a0, text_swap_start
	b		@@text_draw
	nop
	
@@text_draw:

	jal		DrawText_centered
	li		a1, 0xC8
	
	la		a0, text_options
	jal		DrawText_centered
	li		a1, 0xD0
	
	la		a0, 0x00100010
	la		a1, 0x00D00120
	jal		sort_rect
	move	a2, r0
	
	b		@@draw_done
	nop
	
@@skip_banner:
	
	bne		v0, 1, @@draw_done
	nop
	
	la		a2, text
	li		a0, 16
	jal		DrawText
	li		a1, 0x10
	
	la		a2, copyright
	li		a0, 16
	jal		DrawText
	li		a1, 0x18
	
	
	; Generate feature string
	
	sb		r0, 0(s7)
	
	la		v0, config
	lbu		v0, SET_tty(v0)
	nop
	beqz	v0, @@no_stty
	nop
	
	move	a0, s7						; STTY
	la		a1, text_option_stty
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_stty:

	la		v0, config
	lbu		v0, SET_except(v0)
	nop
	beqz	v0, @@no_except
	nop
	
	move	a0, s7
	la		a1, text_option_except		; Exception
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_except:

	la		v0, config
	lbu		v0, SET_except(v0)
	nop
	beq		v0, 1, @@is_unlock
	nop
	beq		v0, 2, @@is_swap
	nop
	b		@@no_cd_special
	nop
	
@@is_unlock:

	move	a0, s7
	la		a1, text_option_unlock		; Unlock
	jal		strcat
	addiu	sp, -8
	b		@@no_cd_special
	addiu	sp, 8
	
@@is_swap:

	move	a0, s7
	la		a1, text_option_swap		; Swap
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_cd_special:

	la		v0, config
	lbu		v0, SET_carttype(v0)
	nop
	bne		v0, 2, @@no_xplorer
	nop
	
	move	a0, s7
	la		a1, text_option_xpdl		; Xplorer download
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_xplorer:

	
	move	a2, s7
	li		a0, 16
	jal		DrawText
	li		a1, 0x20
	
	la		a0, 0x000E000E
	la		a1, 0x001B00DB
	jal		sort_rect
	move	a2, r0
	
	b		@@draw_done
	nop

@@draw_done:

	jal		frame_done
	move	a0, r0
	
	
	la		v0, config
	lbu		v0, SET_carttype(v0)	; Skip Xplorer stuff if not set for Xplorer
	nop
	bne		v0, 2, @@skip_xp
	nop
	
	lbu		v0, VAR_xpready(gp)
	nop
	beqz	v0, @@xp_ready
	nop
	
	lui		a0, XP_IOBASE			; Check data and handshake
	lbu		v1, XP_pcshake(a0)
	lbu		v0, XP_data(a0)
	andi	v1, 0x1
	add		v0, v1
	bnez	v0, @@skip_xp
	nop
	
	b		@@skip_xp				; Clear ready value if handshake
	sb		r0, VAR_xpready(gp)		; and data is zeroed
	
@@xp_ready:

	lui		a0, XP_IOBASE			; Check if handshake went high
	lbu		v0, XP_pcshake(a0)
	nop
	andi	v0, 0x1
	beqz	v0, @@skip_xp
	nop
	
	lbu		v0, XP_data(a0)			; Check if data is set to the correct value
	nop
	beq		v0, 'E', do_xploader
	nop
	beq		v0, 'B', do_xpbinloader
	nop

@@skip_xp:

	
	addiu	a0, gp, VAR_siobuff		; SIO PS-EXE loader
	la		a1, str_mexe
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	beqz	v0, do_loader
	nop
	
	addiu	a0, gp, VAR_siobuff		; SIO binary loader
	la		a1, str_mbin
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, VAR_sioread(gp)
	beqz	v0, do_binloader
	nop
	
	addiu	a0, gp, VAR_siobuff		; SIO patch loader
	la		a1, str_mpat
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, VAR_sioread(gp)
	beqz	v0, do_patloader
	nop
	
	bnez	a0, @@skip_sio_reset
	nop
	
	addiu	a0, gp, VAR_siobuff
	jal		sioSetRead
	addiu	a1, r0, 4
		
@@skip_sio_reset:
	
		
	addiu	a1, gp, VAR_padbuff
	addiu	a2, r0, 34
	jal		PadRead
	move	a0, r0
	
	lhu		v0, VAR_padbuff(gp)
	li		at, 0x4100
	bne		v0, at, @@no_pad
	nop
	lhu		v0, VAR_padbuff+2(gp)
	nop
	andi	v1, v0, 8				; Start button
	beqz	v1, @@exit_boot
	andi	v1, v0, 1				; Select button
	beqz	v1, @@exit_menu
	nop
	
@@no_pad:
	
	b		@@loop
	nop
	
@@exit_boot:
	
	b		@@exit
	move	v0, r0

@@exit_menu:
	
	li		v0, 1
	
@@exit:

	addiu	sp, 32
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
do_xploader:
	jal		xp_readbyte				; To flush the PS-EXE download command
	nop
	jal		xplorer_loader
	nop
	b		back_to_menu
	nop
	
do_xpbinloader:
	jal		xp_readbyte
	nop
	jal		xplorer_binloader
	nop
	b		back_to_menu
	nop
	
do_loader:
	jal		sio_loader
	nop
	b		back_to_menu
	nop
	
do_binloader:
	jal		sio_binloader
	move	a0, r0
	b		back_to_menu
	nop
	
do_patloader:
	jal		sio_binloader
	addiu	a0, r0, 1
	b		back_to_menu
	nop
	
	
sioSetRead:
	; a0 - Target address
	; a1 - Bytes to read
	sw		a0, VAR_sioaddr(gp)
	sw		a1, VAR_sioread(gp)
	
	jr		ra
	nop
	
sioReadWait:
	lw		a0, VAR_sioread(gp)
	nop
	bnez	a0, sioReadWait
	nop
	jr		ra
	nop
	
	
reg2hex:
	; a0 - value
	; a1 - buffer output
	
	la		a2, hex_tbl
	move	t0, r0
	addiu	a1, 7
	sb		r0, 1(a1)
	
@@hex_loop:

	andi	a3, a0, 0xF
	addu	v1, a2, a3
	lbu		v1, 0(v1)
	srl		a0, 4
	sb		v1, 0(a1)
	addiu	a1, -1
	blt		t0, 7, @@hex_loop
	addi	t0, 1
	
	jr		ra
	nop


.include "pad.inc"
.include "cd.inc"
.include "gfx.inc"
.include "rom_menu.inc"
.include "boot.inc"
.include "gpu.inc"
.include "bios.inc"
.include "tim.inc"
.include "eeprom.inc"
.include "sio_loader.inc"
.include "xp_loader.inc"
.include "plasma.inc"
.include "colorbars.inc"


;; Jump functions for calling functions residing in ROM
sioInit:
	la		v0, sioInit_rom
	jr		v0
	nop
	
sioSendByte:
	la		v0, sioSendByte_rom
	jr		v0
	nop
	
sioReadByte:
	la		v0, sioReadByte_rom
	jr		v0
	nop
	

;; Interrupt handler
int_handler:
	
	lw		s0, ISTAT(s7)
	nop
	sw		r0, ISTAT(s7)

	andi	v0, s0, 0x100			;; Serial IRQ handling
	beqz	v0, @@no_sio_irq
	nop

@@sio_wait:
	la		v0, SIO_STAT_REG_A	; Read value from serial
	lw		v0, 0(v0)
	nop
	andi	v0, 0x2
	beqz	v0, @@sio_wait
	nop
	la		v0, SIO_TXRX_REG_A
	lbu		v0, 0(v0)
	
	lw		a0, VAR_sioaddr(gp)
	lw		a1, VAR_sioread(gp)
	
	beqz	a0, @@skip_sio
	nop
	beqz	a1, @@skip_sio
	nop
	
	sb		v0, 0(a0)
	addiu	a0, 1
	addiu	a1, -1
	sw		a0, VAR_sioaddr(gp)
	sw		a1, VAR_sioread(gp)
	
@@skip_sio:
	
	lui		a3, 0x1F80
	lhu		v0, SIO_CTRL_REG(a3)	; Acknowledge serial IRQ
	nop
	ori		v0, 0x10
	sh		v0, SIO_CTRL_REG(a3)
	
@@no_sio_irq:

	
	andi	v0, s0, 0x1				;; VSync IRQ handling
	beqz	v0, @@no_vsync_irq
	nop
	
	lw		v0, VAR_vsync(gp)
	nop
	addiu	v0, 1
	sw		v0, VAR_vsync(gp)
	
@@no_vsync_irq:


	andi	v0, s0, 0x4				;; CD IRQ handling
	beqz	v0, @@no_cd_irq
	nop
	
	li		v0, 1					; Get IRQ number
	sb		v0, CD_REG0(s7)
	lbu		v0, CD_REG3(s7)
	lbu		v1, CD_REG0(s7)
	sb		v0, VAR_cdival(gp)
	
	andi	v1, 0x20				; Get response data
	beqz	v1, @@no_resp
	nop
	addiu	a0, gp, VAR_cdresp
@@resp_loop:
	lbu		v1, CD_REG1(s7)
	lbu		v0, CD_REG0(s7)
	sb		v1, 0(a0)
	andi	v0, 0x20
	bnez	v0, @@resp_loop
	addiu	a0, 1
@@no_resp:
	
	lui		a3, 0x1F80				; Clear CD interrupt and parameter FIFO
	li		v0, 1
	sb		v0, CD_REG0(a3)
	li		v0, 0x5f
	sb		v0, CD_REG3(a3)
	li		v0, 0x40
	sb		v0, CD_REG3(a3)

	li		v0, 0					; Delay when clearing parameter FIFO
	sw		v0, 0(r0)
	li		v0, 1
	sw		v0, 0(r0)
	li		v0, 2
	sw		v0, 0(r0)
	li		v0, 3
	sw		v0, 0(r0)
	
	lbu		v0, VAR_cdinum(gp)		; Increment IRQ count
	nop
	addiu	v0, 1
	sb		v0, VAR_cdinum(gp)
	
@@no_cd_irq:
	
	j		ReturnFromException
	nop

gp_start:
program_end:

.close



.create "romhead.bin", 0x1f000000	;; ROM header and start code

header:			; ROM header
	
	; Postboot vector
	
	.word	dummy
	.ascii	"Licensed by Sony Computer Entertainment Inc."
	.ascii 	"Not officially licensed or endorsed by Sony Computer Entertainment Inc."
	
	.align	0x80					; This keeps things in the header aligned
	
	; Preboot vector
	
	.word	preboot
	.ascii	"Licensed by Sony Computer Entertainment Inc."
	.ascii 	"n00bROM "
	.ascii	version
	.asciiz	" by Lameguy64 of Meido-Tek Productions"
	
	.align	0x80					; This keeps things in the header aligned

	
; Configuration block, aligned to 256 byte boundary
; and padded as a 256 byte block
.align		0x100

config:
	db		0					; Video mode
	db		0					; CD-ROM boot mode
	db		0					; TTY redirect
	db		0					; Exception handler enable
	db		0					; Switch type
	db		0					; Off switch action
	db		0					; Background
	db		0					; Home screen

.align		0x100				; Align to 256 byte block boundary

	
preboot:		; Preboot code (installs break vector and sets breakpoint)

	la		a0, config
	lbu		a0, SET_carttype(a0)
	nop
	beqz	a0, @@no_switch
	nop
	beq		a0, 1, @@switch_par
	nop
	
	lui		a0, 0x1F06			; Xplorer switch detection
	lbu		a0, 0(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@return_bios
	nop
	
	b		@@no_switch
	nop
	
@@switch_par:					; PAR/GS switch detection

	lui		a0, 0x1F02
	lbu		a0, 0x18(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@return_bios
	nop
	
@@no_switch:

	li		v0, BRK_vector		; Install breakpoint vector
	
	li		a0, 0x3c1a1f00		; lui k0, 0x1f00
	sw		a0, 0(v0)
	la		a1, start			; ori k0, < address to ROM entrypoint >
	andi	a1, 0xffff
	lui		a0, 0x375a
	or		a0, a1
	sw		a0, 4(v0)
	li		a0, 0x03400008		; jr  k0
	sw		a0, 8(v0)
	sw		$0, 12(v0)			; nop
	
	lui		v0, 0xffff			; Set BPCM and BDAM masks
	ori		v0, 0xffff
	mtc0	v0, BDAM
	mtc0	v0, BPCM
	
	li		v0, BRK_addr		; Set breakpoint address
	mtc0	v0, BDA
	mtc0	v0, BPC
	
	lui		v0, 0xeb80			; Enable break on pc and memory access
	mtc0	v0, DCIC			; (break on pc for compatibility with no$psx)
	
	jr		ra
	nop
	
@@return_bios:

	la		v0, config			; Continue ROM hook if off switch action
	lbu		v0, SET_offact(v0)	; is set to quick boot
	nop
	beq		v0, 1, @@no_switch
	nop

	jr		ra					; Return to BIOS bootcode (if switch is off)
	nop
	

dummy:			; Dummy vector for postboot
	jr		ra
	nop
	
	
start:			; ROM start code (executed by break vector)

	la		a0, program_pos		; Copy n00bROM code to RAM
	lw		a1, 0(a0)
	lw		a2, 4(a0)
	
	addiu	a0, 8
	addu	a2, a0
	
@@copy_loop:
	lw		v0, 0(a0)
	addiu	a0, 4
	sw		v0, 0(a1)
	ble		a0, a2, @@copy_loop
	addiu	a1, 4
	
	la		a0, program_pos		; Jump execution to copied n00bROM code
	lw		a0, 0(a0)
	nop
	jr		a0
	nop

; Strings and program data stay in ROM

text:
.ascii "n00bROM "
.ascii version
.asciiz " by Lameguy64"
	
copyright:
.asciiz "2020 Meido-Tek Productions"

build_string:
.ascii "Build "
.asciiz build_date
.align 4

.include "exhandler.inc"
.include "siodev.inc"
.include "strings.inc"
.include "tables.inc"

font_tim:
.incbin "font.tim"

.align 4

program_pos:

.close
