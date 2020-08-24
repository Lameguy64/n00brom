.psx

version			equ "0.28b"

.include "cop0regs.inc"
.include "hwregs.inc"
.include "structs.inc"


VAR_vsync		equ	0
VAR_clut		equ	4
VAR_db			equ	8
VAR_ot			equ	12
VAR_paddr		equ	16
VAR_cdcmd		equ	20				; CD sync stuff
VAR_cdiwait		equ	21
VAR_cdinum		equ	22
VAR_cdival		equ	23
VAR_cdresp		equ	24				; CD response buffer
VAR_psexe		equ	40
VAR_padbuff		equ	72
VAR_siobuff		equ	140
VAR_sioaddr		equ	148
VAR_sioread		equ	152
VAR_xpready		equ	156
VAR_pbuff		equ	160


SET_vmode		equ	0				; Video mode
SET_bmode		equ	1				; CD-ROM boot mode
SET_tty			equ	2				; TTY enable
SET_pcdrv		equ	3				; PCDRV enable
SET_except		equ	4				; Exception handler
SET_carttype	equ	5				; Cartridge type
SET_offact		equ	6				; Off switch action
SET_bg			equ	7				; Background
SET_homemode	equ	8				; Home screen mode

BRK_vector		equ	0xa0000040		; Break vector address
BRK_addr		equ	0x80030000		; Breakpoint address (both pc and data break)
PROG_addr		equ	0x801EFFF0		; n00bROM RAM code target address

.macro rfe
	.word	0x42000010
.endmacro

.macro EnterCriticalSection
	addiu	a0, r0, 0x1
	syscall	0
.endmacro

.macro ExitCriticalSection
	addiu	a0, r0, 0x2
	syscall	0
.endmacro


.create "ramprog.bin", PROG_addr-8	; RAM resident code

	dw		PROG_addr				; Simple header for copy routine in ROM
	dw		(program_end-program_start)
	
program_start:
	
	addiu	sp, -4
	sw		ra, 0(sp)
	
	addiu	sp, -INT_size			; INT hook entry structure
	move	s0, sp
	addiu	s1, sp, -4				; INT hook stack
	addiu	sp, -1024
	
	la		gp, gp_start			; GP memory area (end of RAM code)
	
	mtc0	r0, DCIC				; Clear debug control register - not needed
	
	la		a0, int_handler			; Create a HookEntryInt hook structure
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
	
	move	a0, s0					; Hook custom IRQ handler
	jal		HookEntryInt
	addiu	sp, -4
	addiu	sp, 4
	
	move	a0, r0					; Disable VSync IRQ auto acknowledge
	jal		ChangeClearPAD
	addiu	sp, -4
	addiu	sp, 4
	
	li		a0, 3					; Disable root counter IRQ auto ack
	move	a1, r0
	jal		ChangeClearRCnt
	addiu	sp, -8
	addiu	sp, 8
	
	lui		a0, 0x1F80				; Enable VSync IRQ
	li		v0, 0x5
	sw		v0, IMASK(a0)
	
	sw		r0, VAR_vsync(gp)		; Clear a number of global variables
	sw		r0, VAR_paddr(gp)
	sb		r0, VAR_db(gp)
	sw		r0, VAR_cdcmd(gp)
	
	sw		r0, VAR_sioaddr(gp)		; Clear SIO buffers
	sw		r0, VAR_sioread(gp)
	
	lui		v0, 0x3b33				; Enables DMA channel 6 (for ClearOTag)
	ori		v0, 0x3b33				; Enables DMA channel 2 (for GPU)
	sw		v0, DPCR(a0)
	sw		r0, DICR(a0)			; Clear DICR (not needed)
	
	la		a0, install_syserr		; Install syserr function hooks
	jalr	a0
	nop
	
	la		v1, config
	lbu		v1, SET_except(v1)
	
	la		a0, install_exception	; Don't install exception handler if not
	beqz	v1, @@no_exception		; enabled in settings
	nop
	jalr	a0						; Install exception handler
	nop
	
@@no_exception:

	la		v0, config				; Skip Xplorer stuff if not set for Xplorer
	lbu		v0, SET_carttype(v0)
	nop
	bne		v0, 2, @@skip_xplorer
	nop
	
	lui		a0, XP_IOBASE			; Xplorer prep stuff
	lbu		v0, XP_pcshake(a0)		; Keep last PC handshake state
	sb		r0, XP_latch(a0)		; Clear latch
	andi	v0, 0x1
	sb		v0, VAR_xpready(gp)
	sb		r0, XP_latch(a0)
	
@@skip_xplorer:

	la		v0, config				; Install pcdrv device for Xplorer
	lbu		v0, SET_pcdrv(v0)
	nop
	beqz	v0, @@skip_pcdrv
	nop
	
	la		a0, install_xp_pcdrv
	jalr	a0
	nop
	
@@skip_pcdrv:

	jal		main					; Jump to main
	rfe
	
	EnterCriticalSection			; Exit routine (to BIOS bootstrap routine)

	jal		HookDefaultInt
	nop
	
	addiu	a0, r0, 1				; Enable autoclear for pad and root counter
	jal		ChangeClearPAD
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 3
	addiu	a1, r0, 1
	jal		ChangeClearRCnt
	addiu	sp, -8
	addiu	sp, 8
	
	addiu	sp, INT_size
	addiu	sp, 1024
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	

main:

	addiu	sp, -4
	sw		ra, 0(sp)
	
	jal		init					; do init
	nop
	
	la		a0, config				; Get switch setting
	lbu		a0, SET_carttype(a0)
	nop
	beqz	a0, @@return_to_idle
	nop
	beq		a0, 1, @@switch_par
	nop
	lui		a0, 0x1F06				; Xplorer switch detection
	lbu		a0, 0(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@do_quickboot
	nop
	b		@@return_to_idle
	nop
@@switch_par:						; PAR/GS switch detection
	lui		a0, 0x1F02
	lbu		a0, 0x18(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@do_quickboot
	nop	
	
@@return_to_idle:

	jal		idle_screen
	nop

	move	a0, r0					; Stop SIO receive
	jal		sioSetRead
	move	a1, r0

	beqz	v0, @@boot
	nop
	
	jal		rom_menu				; Go to ROM menu
	nop
	
	beqz	v0, @@return_to_idle
	nop
	
	beq		v0, 2, flash_mode		; Enter self-flash mode
	nop
	
	EnterCriticalSection			; Soft reset
	
	lui		a0, 0xBFC0
	jr		a0
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
	
@@boot:

	jal		boot_setup
	nop
	beqz	v0, @@return_to_idle
	nop
	
@@exit:

	la		a0, text_booting
	jal		sort_status
	nop

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	

boot_setup:							; Performs swap trick or unlock depending
									; on configuration
	addiu	sp, -4
	sw		ra, 0(sp)
	
	la		v0, config
	lbu		v0, SET_bmode(v0)
	nop
	beq		v0, 1, @@unlock_boot
	nop
	beq		v0, 2, @@swap_boot
	nop
	b		@@return
	addiu	v0, r0, 1

@@unlock_boot:
	jal		init_disc
	nop
	jal		unlock_cdrom
	nop
	b		@@return
	nop
	
@@swap_boot:
	jal		init_disc
	nop
	jal		swap_trick
	nop

@@return:

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
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
	
	move	a0, r0					; Disable VSync IRQ auto acknowledge
	jal		ChangeClearPAD
	addiu	sp, -4
	addiu	sp, 4
	
	
	jal		sioInit					; Initialize serial
	nop

	la		a0, config				; Install n00bROM TTY device if enabled
	lbu		v0, SET_tty(a0)
	nop
	beqz	v0, @@no_tty
	nop
	beq		v0, 2, @@xp_tty
	nop
	
	la		a0, InstallTTY_sio		; Install serial TTY
	jalr	a0
	nop
	
	b		@@no_tty
	nop
	
@@xp_tty:

	la		a0, InstallTTY_xp		; Install Xplorer TTY
	jalr	a0
	nop
	
@@no_tty:

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
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

	
idle_screen:

	addiu	sp, -4
	sw		ra, 0(sp)

	addiu	sp, -32
	move	s7, sp
	
	lw		s0, VAR_vsync(gp)
	nop
	srl		s0, 2
	
	sw		r0, VAR_siobuff(gp)		; Clear SIO temp buffer
	sw		r0, VAR_siobuff+4(gp)
	
	addiu	a0, gp, VAR_siobuff		; Set read target
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
		
	lw		v0, VAR_vsync(gp)		; Screen saver that activates in 2 minutes
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
	
	sb		r0, 0(s7)				; Create feature string
	
	la		v0, config				; TTY type
	lbu		v0, SET_tty(v0)
	nop
	beqz	v0, @@no_stty
	nop

	bne		v0, 2, @@is_stty
	nop
	
	move	a0, s7					; XTTY
	la		a1, text_option_xtty
	jal		strcat
	addiu	sp, -8
	
	b		@@no_stty
	addiu	sp, 8

@@is_stty:
	
	move	a0, s7					; STTY
	la		a1, text_option_stty
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_stty:

	la		v0, config				; Exception
	lbu		v0, SET_except(v0)
	nop
	beqz	v0, @@no_except
	nop
	
	move	a0, s7
	la		a1, text_option_except
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_except:

	la		v0, config				; Boot mode
	lbu		v0, SET_bmode(v0)
	nop
	beq		v0, 1, @@is_unlock
	nop
	beq		v0, 2, @@is_swap
	nop
	b		@@no_cd_special
	nop
	
@@is_unlock:

	move	a0, s7					; Unlock
	la		a1, text_option_unlock
	jal		strcat
	addiu	sp, -8
	b		@@no_cd_special
	addiu	sp, 8
	
@@is_swap:

	move	a0, s7					; Swap trick
	la		a1, text_option_swap
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_cd_special:

	la		v0, config				; Xplorer download
	lbu		v0, SET_carttype(v0)
	nop
	bne		v0, 2, @@no_xplorer
	nop
	
	move	a0, s7
	la		a1, text_option_xpdl
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_xplorer:

	la		v0, config				; PCDRV
	lbu		v0, SET_pcdrv(v0)
	nop
	beqz	v0, @@no_pcdrv
	nop
	
	move	a0, s7
	la		a1, text_option_pcdrv
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
@@no_pcdrv:
	
	move	a2, s7					; Draw the options string
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
	
	la		v0, config				; Skip Xplorer stuff if cart type is not set
	lbu		v0, SET_carttype(v0)	; to Xplorer
	nop
	bne		v0, 2, @@skip_xp
	nop
	
	lbu		v0, VAR_xpready(gp)
	nop
	bnez	v0, @@xp_ready
	nop
	
	lui		a0, XP_IOBASE			; Check data and handshake
	lbu		v0, XP_pcshake(a0)
	;lbu		v0, XP_data(a0)
	nop
	andi	v0, 0x1
	;add		v0, v1
	beqz	v0, @@skip_xp
	nop
	
	addiu	v0, r0, 1
	b		@@skip_xp				; Set ready value if handshake and data is
	sb		v0, VAR_xpready(gp)		; zeroed
	
@@xp_ready:

	lui		a0, XP_IOBASE			; Check if handshake went low
	lbu		v0, XP_pcshake(a0)
	nop
	andi	v0, 0x1
	bnez	v0, @@skip_xp
	nop
	
	lbu		v0, XP_data(a0)			; Check if data is set to the correct value
	nop
	beq		v0, 'E', @@do_xploader
	nop
	beq		v0, 'B', @@do_xpbinloader
	nop
	beq		v0, 'P', @@do_xppatloader
	nop

@@skip_xp:
	
	addiu	a0, gp, VAR_siobuff		; SIO PS-EXE loader
	la		a1, str_mexe
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	beqz	v0, @@do_loader
	nop
	
	addiu	a0, gp, VAR_siobuff		; SIO binary loader
	la		a1, str_mbin
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, VAR_sioread(gp)
	beqz	v0, @@do_binloader
	nop
	
	addiu	a0, gp, VAR_siobuff		; SIO patch loader
	la		a1, str_mpat
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, VAR_sioread(gp)
	beqz	v0, @@do_patloader
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
	
@@do_xploader:
	jal		xp_readbyte				; To flush the PS-EXE download command
	nop
	jal		xplorer_loader
	nop
	b		@@loop
	nop
	
@@do_xpbinloader:
	jal		xp_readbyte
	nop
	jal		xplorer_binloader
	move	a0, r0
	b		@@loop
	nop
	
@@do_xppatloader:
	jal		xp_readbyte
	nop
	jal		xplorer_binloader
	addiu	a0, r0, 1
	b		@@loop
	nop
	
@@do_loader:
	jal		sio_loader
	nop
	b		@@sio_end
	nop
	
@@do_binloader:
	jal		sio_binloader
	move	a0, r0
	b		@@sio_end
	nop
		
@@do_patloader:
	jal		sio_binloader
	addiu	a0, r0, 1
	b		@@sio_end
	nop
	
@@sio_end:
	addiu	a0, gp, VAR_siobuff		; Set read target
	jal		sioSetRead
	addiu	a1, r0, 4
	sw		r0, VAR_siobuff+0(gp)
	sw		r0, VAR_siobuff+4(gp)
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
	
	
sioSetRead:							; a0 - Target address
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
	
	
reg2hex:							; a0 - value
	la		t0, ex_reg2hex			; a1 - buffer output
	jr		t0
	nop
	


.include "pad.inc"		
.include "flash.inc"
.include "cd.inc"
.include "cdtricks.inc"
.include "gfx.inc"
.include "rom_menu.inc"
.include "gpu.inc"
.include "bios.inc"
.include "tim.inc"
.include "eeprom.inc"
.include "sio_loader.inc"
.include "xp_loader.inc"
.include "plasma.inc"
.include "colorbars.inc"


sioInit:							; Jump functions for calling functions that
	la		v0, sioInit_rom			; reside in ROM space
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
	

int_handler:						; Interrupt handler (called by BIOS kernel)
	
	lw		s0, ISTAT(s7)
	nop
	sw		r0, ISTAT(s7)

	andi	v0, s0, 0x100			; Serial IRQ handling
	beqz	v0, @@no_sio_irq
	nop

@@sio_wait:
	la		v0, SIO_STAT_REG_A		; Read value from serial
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

	
	andi	v0, s0, 0x1				; VSync IRQ handling
	beqz	v0, @@no_vsync_irq
	nop
	
	lw		v0, VAR_vsync(gp)
	nop
	addiu	v0, 1
	sw		v0, VAR_vsync(gp)
	
@@no_vsync_irq:


	andi	v0, s0, 0x4				; CD IRQ handling
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


.create "romhead.bin", 0x1f000000	; ROM header and start code

header:								; ROM header
	
	.word	0						; Postboot vector (not used, filled with
	.ascii 	"n00bROM "				; information instead)
	.ascii	version
	.ascii	" by Lameguy64 of Meido-Tek Productions"
	.ascii	" build "
	.asciiz build_date
	.asciiz	"https://github.com/lameguy64/n00brom"
	
	.align	0x80					; Keeps headers aligned
	
	.word	preboot					; Preboot vector
	.ascii	"Licensed by Sony Computer Entertainment Inc."
	.ascii	"Not officially licensed or endorsed by Sony "
	.ascii	"Computer Entertainment"
	.align	0x80					; This keeps things in the header aligned

	
.align		0x100					; Configuration block, aligned to 256 byte
									; boundary and padded as a 256 byte block
config:
	db		0						; Video mode
	db		0						; CD-ROM boot mode
	db		0						; TTY redirect mode
	db		0						; PCDRV enable
	db		0						; Exception handler enable
	db		0						; Switch type
	db		0						; Off switch action
	db		0						; Background
	db		0						; Home screen

.align		0x100					; Pad to align to the 256 byte block
									; boundary for saving options easily

sram_jump:							; Special SRAM jump for use with debuggers

	lui		v0, XP_IOBASE			; Set memory map to SRAM
	li		v1, 0x10
	sb		v1, 1(v0)
	lui		v0, 0x1F04				; Jump to SRAM region
	jr		v0
	nop

preboot:							; Preboot code, installs break vector and 
									; sets breakpoint for midbook trick
								
	la		a0, config				; Switch detection based on cartridge type
	lbu		a0, SET_carttype(a0)
	nop
	beqz	a0, @@no_switch
	nop
	beq		a0, 1, @@switch_par
	nop
	
	lui		a0, 0x1F06				; Xplorer switch detection
	lbu		a0, 0(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@return_bios
	nop
	
	b		@@no_switch
	nop
	
@@switch_par:						; PAR/GS switch detection

	lui		a0, 0x1F02
	lbu		a0, 0x18(a0)
	nop
	andi	a0, 0x1
	beqz	a0, @@return_bios
	nop
	
@@no_switch:

	li		v0, BRK_vector			; Install breakpoint vector
	
	li		a0, 0x3c1a1f00			; lui k0, 0x1f00
	sw		a0, 0(v0)
	la		a1, start				; ori k0, < address to ROM entrypoint >
	andi	a1, 0xffff
	lui		a0, 0x375a
	or		a0, a1
	sw		a0, 4(v0)
	li		a0, 0x03400008			; jr  k0
	sw		a0, 8(v0)
	sw		$0, 12(v0)				; nop
	
	lui		v0, 0xffff				; Set BPCM and BDAM masks
	ori		v0, 0xffff
	mtc0	v0, BDAM
	mtc0	v0, BPCM
	
	li		v0, BRK_addr			; Set breakpoint address
	mtc0	v0, BDA
	mtc0	v0, BPC
	
	lui		v0, 0xeb80				; Enable break on pc and memory access
	mtc0	v0, DCIC				; (break on pc for no$psx compatibility)
	
	jr		ra
	nop
	
@@return_bios:

	la		v0, config				; Continue ROM hook if off switch action
	lbu		v0, SET_offact(v0)		; is set to quick boot
	nop
	beq		v0, 1, @@no_switch
	nop

	jr		ra						; Return to BIOS bootcode (if switch is off)
	nop
	
	
; The first phase of this midboot bootstrap trick is triggered by a data access
; breakpoint on the BIOS shell's target address. The memcpy function used to
; copy the shell is skipped by simply returning from exception to the address 
; in the ra register which exits the function prematurely. This shouldn't be
; much of a problem as the memcpy function usually doesn't use the stack.
;
; The second phase is when the BIOS attempts to jump to the shell's target
; address which will trigger the program counter breakpoint. At this point,
; n00bROM is bootstrapped, but the return address to the BIOS is preserved as
; it is needed for using the BIOS' CD-ROM bootstrap routine initiated by simply
; jumping to that address.

; n00bROM effectively replaces the shell more or less.
	
start:								; ROM start code (executed by break vector)
	
	mfc0	v0, DCIC			
	nop							
	andi	v0, 0x10				; Check if data access breakpoint
	beqz	v0, not_memcpy			; If not, bootstrap n00bROM
	nop
	
	lui		v0, 0xe180				; Break on program counter for second phase
	mtc0	v0, DCIC
	nop
	nop
	jr		ra
	nop
	
not_memcpy:
	
	addiu	sp, -4
	sw		ra, 0(sp)
	
	la		a0, program_pos			; Copy n00bROM code to RAM
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
	
	la		a0, program_pos			; Jump execution to copied n00bROM code
	lw		a0, 0(a0)
	nop
	jalr	a0
	nop
	
	li		a0, 0x8000DFFC			; Make sure s_needsCDRomReset is zero for
	sw		r0, 0(a0)				; swap trick to work (thanks nicolasnoble)
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop

text:								; Strings, tables  and program data stay
.ascii "n00bROM "					; in ROM to keep RAM code small
.ascii version
.asciiz " by Lameguy64"
	
copyright:
.asciiz "2020 Meido-Tek Productions"

build_string:
.ascii "Build "
.asciiz build_date
.align 4

.include "siodev.inc"
.include "xp_tty.inc"
.include "xp_pcdrv.inc"
.include "syserr.inc"
.include "exhandler.inc"
.include "strings.inc"
.include "tables.inc"

font_tim:
.incbin "font.tim"

.align 4

program_pos:

.close
