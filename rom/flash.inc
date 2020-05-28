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
	beqz	v0, @@proceed_download_sio
	nop
	
	lw		v0, VAR_sioread(gp)		; Retry
	nop
	beqz	v0, flash_mode
	nop
	
	la		v0, config				; Xplorer binary loader
	lbu		v0, SET_carttype(v0)
	nop
	bne		v0, 2, @@download_wait
	nop
	
	lbu		v0, VAR_xpready(gp)
	nop
	bnez	v0, @@xp_ready
	nop
	
	lui		a0, XP_IOBASE
	lbu		v0, XP_pcshake(a0)
	nop
	andi	v0, 0x1
	beqz	v0, @@download_wait
	nop
	
	li		v0, 1
	b		@@download_wait
	sb		v0, VAR_xpready(gp)
	
@@xp_ready:

	lui		a0, XP_IOBASE			; Check if handshake went low
	lbu		v0, XP_pcshake(a0)
	nop
	andi	v0, 0x1
	bnez	v0, @@download_wait
	nop
	
	lbu		v0, XP_data(a0)
	nop
	beq		v0, 'B', @@proceed_download_xp
	nop
	
	b		@@download_wait
	nop
	
@@proceed_download_xp:
	jal		xp_readbyte
	nop
	jal		xplorer_binloader
	nop
	beqz	v0, flash_mode			; Retry download if checksum failed
	nop
	b		@@download_ok
	nop
	
@@proceed_download_sio:

	jal		sio_binloader			; Download the ROM image
	li		a0, 2
	beqz	v0, flash_mode			; Retry download if checksum failed
	nop
	
@@download_ok:

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
	
	EnterCriticalSection
	
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
	
	ExitCriticalSection
	
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
