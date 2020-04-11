; Disc boot-up sequence

EXE_pc0		equ	0			; PS-EXE header offsets
EXE_gp0		equ	4
EXE_taddr	equ	8
EXE_tsize	equ	12
EXE_daddr	equ	16
EXE_dsize	equ	20
EXE_baddr	equ	24
EXE_bsize	equ	28
EXE_spaddr	equ	32
EXE_sp_size	equ	36
EXE_sp		equ	40
EXE_fp		equ 44
EXE_gp		equ	48
EXE_ret		equ 52
EXE_base	equ	56
EXE_datapos	equ	60


boot_disc:
	
	la		a0, text_cnf_read
	jal		sort_status
	nop
	
	jal		init_disc_bios			; Initialize BIOS CD-ROM subsystem
	nop
	
	la		a0, s_systemcnf			; Attempt to open the SYSTEM.CNF file on CD
	li		a1, 1
	jal		open
	addiu	sp, -8
	addiu	sp, 8
	
	bltz	v0, @@no_systemcnf		; Fallback to loading PSX.EXE if not found
	nop
	
	move	s4, v0
	move	a0, v0					; Read file contents of SYSTEM.CNF
	addiu	a1, gp, VAR_pbuff
	li		a2, 0x0800
	jal		read
	addiu	sp, -12
	addiu	sp, 12
	
	bltz	v0, @@read_error
	nop
	
	move	v0, s4					; Close file
	jal		close
	addiu	sp, -4
	addiu	sp, 4
	
	; Parse CNF file
	
	la		a1, s_tcb				; Get TCB number
	jal		strcasestr
	addiu	a0, gp, VAR_pbuff
	jal		skipspace				; Skip spaces
	addiu	a0, v0, 3
	addiu	a0, v0, -2				; Step two characters back and inject '0x'
	li		v0, '0'
	sb		v0, 0(a0)
	li		v0, 'x'
	sb		v0, 1(a0)
	jal		atoi
	addiu	sp, -4
	addiu	sp, 4
	move	s1, v0
	
	la		a1, s_evcb				; Get EVCB number
	jal		strcasestr
	addiu	a0, gp, VAR_pbuff
	jal		skipspace
	addiu	a0, v0, 5
	addiu	a0, v0, -2
	li		v0, '0'
	sb		v0, 0(a0)
	li		v0, 'x'
	sb		v0, 1(a0)
	jal		atoi
	addiu	sp, -4
	addiu	sp, 4
	move	s0, v0
	
	la		a1, s_stack				; Get STACK address
	jal		strcasestr
	addiu	a0, gp, VAR_pbuff
	jal		skipspace
	addiu	a0, v0, 5
	addiu	a0, v0, -2
	li		v0, '0'
	sb		v0, 0(a0)
	li		v0, 'x'
	sb		v0, 1(a0)
	jal		atoi
	addiu	sp, -4
	addiu	sp, 4
	move	s2, v0
	
	la		a1, s_boot				; Get the PS-EXE file name
	jal		strcasestr
	addiu	a0, gp, VAR_pbuff
	
	jal		skipspace				; Skip spaces
	addiu	a0, v0, 4
	
	addiu	a0, gp, VAR_psexe		; Extract the line
	jal		getline
	move	a1, v0
	
	b		@@do_load				; Proceed loading PS-EXE
	addiu	s3, gp, VAR_psexe
	
@@no_systemcnf:						; Load fallback

	li		s0, 0x10				; Default EvCBs and TCBs
	li		s1, 0x04
	li		s2, SP_base				; Default stack
	la		s3, s_psxexe			; Attempt loading PSX.EXE
	
@@do_load:

	la		a0, text_booting
	jal		sort_status
	nop
	
	EnterCriticalSection

	lui		a0, 0x1F80				; Only leave CD and DMA IRQ enabled
	lw		v0, IMASK(a0)
	nop
	
	andi	v0, 0xC
	sw		v0, IMASK(a0)
	sw		r0, ISTAT(a0)
	jal		HookDefaultInt			; Set kernel default IRQ handler
	nop
	
	addiu	a0, r0, 1
	jal		ChangeClearPAD
	addiu	sp, -4
	addiu	sp, 4
	
	addiu	a0, r0, 3
	addiu	a1, r0, 1
	jal		ChangeClearRCnt
	addiu	sp, -8
	addiu	sp, 8
	
	ExitCriticalSection
	
	move	a0, s3					; Load executable
	move	a1, s2
	move	a2, r0
	jal		LoadExec
	addiu	sp, -12
	addiu	sp, 12
	
@@loop:
	b		@@loop
	nop
	
@@read_error:
	la		a0, text_cnf_read_error
	jal		sort_status
	nop
	b		@@read_error
	nop


init_disc_bios:
	addiu	sp, -8
	sw		ra, 0(sp)
	
	lui		a0, 0x1F80				; Load IOBASE value
	
	lw		v0, DPCR(a0)			; Get current DMA settings
	nop
	sw		v0, 4(sp)				; Save to stack
	
	jal		_96_init				; Init CD subsystem
	nop
	
	lui		a0, 0x1F80				; Load IOBASE again
	
	lw		v0, 4(sp)				; Get old DMA control settings
	lw		v1, DPCR(a0)			; Get DMA settings by _96_init()
	
	lui		a1, 0xffff				; Mask out settings for CD DMA
	ori		a1, 0x0f00
	and		v0, a1
	
	or		v0, v1					; Merge and set new DMA settings
	sw		v0, DPCR(a0)
	
	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	nop
	
	
; Skips spaces and equal signs (used by CNF parser)
skipspace:
	; a0 - Input string
	; Return: v0 - Address to first non-space character.
	lbu		v1, 0(a0)
	nop
	beq		v1, ' ', @@skip
	nop
	beq		v1, '=', @@skip
	nop
	jr		ra
	move	v0, a0
@@skip:
	b		skipspace
	addiu	a0, 1
	

; Copies a string until a CR/LF or space is encountered
getline:
	; a0 - Output address
	; a1 - String to copy from
	lbu		v0, 0(a1)
	addiu	a1, 1
	beqz	v0, @@end
	nop
	beq		v0, 0x0D, @@end
	nop
	beq		v0, 0x0A, @@end
	nop
	beq		v0, ' ', @@end
	nop
	sb		v0, 0(a0)
	b		getline
	addiu	a0, 1
@@end:
	jr		ra
	sb		r0, 0(a0)

	
; strcasestr implementation
strcasestr:
	; a0 - String to search
	; a1 - String to find
	addiu	sp, -24
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	sw		a1, 16(sp)
	
@@scan_start:

	sw		a0, 12(sp)
	
@@comp_loop:
	
	lbu		s0, 0(a0)				; Load character from A and B
	lbu		s1, 0(a1)
	
	beqz	s0, @@end_strcasestr
	nop
	beqz	s1, @@found
	nop
	
	sw		a0, 20(sp)				; Save a0 parameter
	
	move	a0, s0
	jal		toupper					; tolower character A
	addiu	sp, -4
	addiu	sp, 4
	move	s0, v0
	
	move	a0, s1
	jal		toupper					; tolower character B
	addiu	sp, -4
	addiu	sp, 4
	move	s1, v0
	
	lw		a0, 20(sp)				; Restore a0 parameter
	
	addiu	a1, 1
	addiu	a0, 1
	
	beq		s0, s1, @@comp_loop		; If value matches continue compare
	nop

@@end_strcasestr:

	lw		a0, 12(sp)				; Rescan from next character of string A
	lw		a1, 16(sp)
	addiu	a0, 1
	
	lbu		v0, 0(a0)
	nop
	beqz	v0, @@not_found			; If terminator is reached, string is not found
	nop
	
	b		@@scan_start
	nop

@@not_found:
	
	b		@@quit
	move	v0, r0
	
@@found:

	lw		v0, 12(sp)				; Return address of string match
	
@@quit:

	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	jr		ra
	addiu	sp, 24


unlock_cdrom:						;; HC05 secret unlock boot method

	addiu	sp, -4
	sw		ra, 0(sp)
	
	la		a0, text_unlock_cd
	jal		sort_status
	nop

	sw		r0, VAR_cdresp(gp)
	sw		r0, VAR_cdresp+4(gp)
	sw		r0, VAR_cdresp+8(gp)
	sw		r0, VAR_cdresp+12(gp)

	li		a0, 0x19
	la		a1, getid_cmd
	jal		cd_control
	li		a2, 1
	jal		cd_wait
	li		a0, 1

	addiu	a0, gp, VAR_cdresp		; Is region string US?
	la		a1, s_uscan_chk
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	
	bnez	v0, @@not_us
	nop
	b		@@is_compatible
	li		v0, 0
	
@@not_us:

	addiu	a0, gp, VAR_cdresp		; Is region string EU?
	la		a1, s_europe_chk
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8

	bnez	v0, @@not_eu
	nop
	b		@@is_compatible
	li		v0, 1
	
@@not_eu:
	
	addiu	a0, gp, VAR_cdresp		; Is region string Worldwide (Yaroze)?
	la		a1, s_netna_chk
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	
	bnez	v0, @@unlock_not_support
	nop
	
	li		v0, 2

@@is_compatible:
	
	addiu	sp, -4
	sw		v0, 0($sp)
	
	li		a0, 0x50				; Unlock pt1
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	la		a0, s_unlock_a			; Unlock pt2
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x51
	la		a1, s_unlock_a
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1
	
	la		a0, s_unlock_b			; Unlock pt3
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x52
	la		a1, s_unlock_b
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1
	
	la		a0, s_unlock_c			; Unlock pt4
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x53
	la		a1, s_unlock_c
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1

	la		a0, s_unlock_d			; Unlock pt5
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x54
	la		a1, s_unlock_d
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1
	
	lw		v0, 0(sp)
	addiu	sp, 4
	
	beq		v0, 0, @@unlock_us
	nop
	beq		v0, 1, @@unlock_eu
	nop
	
	la		a0, s_unlock_e_ww			; Unlock Worldwide (yaroze)
	jal		strlen
	addiu	sp, -4
	la		a1, s_unlock_e_ww
	b		@@unlock_end
	addiu	sp, 4
	
@@unlock_us:

	la		a0, s_unlock_e_us			; Unlock US
	jal		strlen
	addiu	sp, -4
	la		a1, s_unlock_e_us
	b		@@unlock_end
	addiu	sp, 4

@@unlock_eu:

	la		a0, s_unlock_e_eu			; Unlock EU
	jal		strlen
	addiu	sp, -4
	la		a1, s_unlock_e_eu
	b		@@unlock_end
	addiu	sp, 4
	
@@unlock_end:

	li		a0, 0x55
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1

	li		a0, 0x56					; Last unlock command
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1

	lw		ra, 0(sp)					; Return with non-zero value
	addiu	sp, 4
	jr		ra							
	addiu	v0, r0, 1
	
@@unlock_not_support:					; If unlock not supported (Japan)

	jal		prep_frame
	nop
	
	la		a0, text_unlock_incompat
	jal		DrawText_multiline
	li		a1, 0x5c
	
	la		a0, text_return
	jal		DrawText_centered
	li		a1, 0x8c
	
	la		a0, 0x00500020
	la		a1, 0x00500100
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
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
	andi	v0, 0x4000				; Cross button
	beqz	v0, @@exit
	nop
	
@@no_pad:
	
	b		@@unlock_not_support
	nop
	
@@exit:
	
	lw		ra, 0(sp)				; Return with non-zero value
	addiu	sp, 4
	jr		ra							
	move	v0, r0
	

swap_trick:							;; Ez-swap boot method

	addiu	sp, -4
	sw		ra, 0(sp)
	
@@repeat_getid:

	li		a0, 0x1A				; GetID
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	lbu		v0, VAR_cdival(gp)		; Repeat command if first response is INT5
	nop
	andi	v0, 0x7
	beq		v0, 0x5, @@repeat_getid
	nop
	
	jal		cd_wait					; Wait for second response
	li		a0, 2
	
	lbu		v0, VAR_cdival(gp)
	nop
	andi	v0, 0x7
	beq		v0, 0x5, swap_no_disc
	nop
	
	lbu		v0, VAR_cdresp+2(gp)	; Check if a Audio CD was stuffed
	nop								; on a chipped console
	beqz	v0, swap_no_disc
	nop
	
	li		a0, 0x08				; Stop
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 2
	
disc_stopped:

	addiu	a1, gp, VAR_padbuff
	addiu	a2, r0, 34
	jal		PadRead
	move	a0, r0
	
	lhu		v0, VAR_padbuff(gp)
	li		at, 0x4100
	bne		v0, at, @@loop
	nop
	lhu		v0, VAR_padbuff+2(gp)
	nop
	andi	v0, 8					; Start button
	beqz	v0, disc_stopped
	nop
	
@@loop:

	jal		prep_frame
	nop
	
	la		a0, text_swap
	jal		DrawText_multiline
	li		a1, 0x60
	
	la		a0, text_continue
	jal		DrawText_centered
	li		a1, 0x80
	
	la		a0, 0x00500020
	la		a1, 0x00500100
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
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
	andi	v0, 8					; Start button
	beqz	v0, swap_start_disc
	nop
	
@@no_pad:
	
	b		@@loop
	nop
	
swap_start_disc:
	
	li		a0, 0x07				; MotorOn
	jal		cd_control
	move	a2, r0
	
@@loop:

	la		a0, text_check_cd
	jal		sort_status
	nop

	lbu		v0, VAR_cdinum(gp)		; Wait until two CD IRQs occur on init
	nop
	blt		v0, 2, @@loop
	nop
	
@@repeat_getid:

	li		a0, 0x1A				; GetID
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	lbu		v0, VAR_cdival(gp)		; Repeat command if first response is INT5
	nop
	andi	v0, 0x7
	beq		v0, 0x5, @@repeat_getid
	nop
	
	jal		cd_wait					; Wait for second response
	li		a0, 2
	
	lbu		v0, VAR_cdival(gp)
	nop
	andi	v0, 0x7
	beq		v0, 0x5, swap_no_disc
	nop

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	addi	v0, r0, 1
	
swap_no_disc:
	
	addiu	a1, gp, VAR_padbuff
	addiu	a2, r0, 34
	jal		PadRead
	move	a0, r0
	
	lhu		v0, VAR_padbuff(gp)
	li		at, 0x4100
	bne		v0, at, @@loop
	nop
	lhu		v0, VAR_padbuff+2(gp)
	nop
	andi	v0, 8					; Start button
	beqz	v0, swap_no_disc
	nop
	
@@loop:

	jal		prep_frame
	nop
	
	la		a0, text_no_disc
	jal		DrawText_centered
	li		a1, 0x6c
	
	la		a0, text_retry
	jal		DrawText_centered
	li		a1, 0x7c
	
	la		a0, 0x00500020
	la		a1, 0x00500100
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
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
	andi	v0, 8					; Start button
	beqz	v0, init_disc
	nop
@@no_pad:
	
	b		@@loop
	nop


getid_cmd:
.byte	0x22
.align 4
