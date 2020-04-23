sio_loader:							;; Loader routine

	addiu	sp, -4
	sw		ra, 0(sp)
	
	subiu	sp, EXEC_len
	move	s0, sp
	
	move	a0, s0					; Prepare receiving parameters
	jal		sioSetRead
	li		a1, EXEC_len
	
	jal		sioSendByte				; Response byte to receive EXEC parameters
	li		a0, 'K'
	
	jal		sioReadWait
	nop
	
	lw		a0, EXEC_taddr(s0)		; Download EXE body
	lw		a1, EXEC_tsize(s0)
	jal		sioSetRead
	nop
	
@@recv_wait_2:						; Wait until receive is complete
	la		a0, text_download
	jal		sort_status
	nop
	lw		a0, VAR_sioread(gp)
	nop
	bgtz	a0, @@recv_wait_2
	nop
	
	lw		a0, EXEC_taddr(s0)
	lw		a1, EXEC_tsize(s0)
	jal		checksum
	addiu	a2, r0, -1
	
	lw		v1, EXEC_crc32(s0)
	nop
	bne		v0, v1, @@checksum_fail
	nop
	
	la		a0, text_exec
	jal		sort_status
	nop
	
	j		do_loader_exec
	nop

@@checksum_fail:

	move	s0, r0

@@message_loop:

	jal		prep_frame
	nop
	
	la		a0, text_csum_fail
	jal		DrawText_centered
	li		a1, 0x7c
	
	la		a0, 0x00500020
	la		a1, 0x00500100
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
	blt		s0, 120, @@message_loop
	addi	s0, 1
	
	addiu	sp, EXEC_len
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
sio_binloader:
	
	addiu	sp, -(8+BIN_len)
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	
	addiu	s0, sp, 8
	
	move	a0, s0					; Prepare receiving parameters
	jal		sioSetRead
	li		a1, BIN_len
	
	jal		sioSendByte				; Response byte to receive BIN parameters
	li		a0, 'K'
	
	jal		sioReadWait
	nop
	
	lw		v0, 4(sp)				; Use fixed address if patch mode
	lui		a0, 0xA001
	bnez	v0, @@patch_addr
	nop
	lw		a0, BIN_addr(s0)		; Download binary data
@@patch_addr:
	lw		a1, BIN_size(s0)
	jal		sioSetRead
	nop
	
@@recv_wait_2:						; Wait until receive is complete
	lw		v0, 4(sp)
	la		a0, text_download_bin
	bne		v0, 1, @@not_patch
	nop
	la		a0, text_download_pat
@@not_patch:
	bne		v0, 2, @@not_rom
	nop
	la		a0, text_download_rom
@@not_rom:
	
	jal		sort_status
	nop
	lw		a0, VAR_sioread(gp)
	nop
	bgtz	a0, @@recv_wait_2
	nop
	
	lw		v0, 4(sp)
	lw		a0, BIN_addr(s0)
	beqz	v0, @@use_addr
	nop
	lui		a0, 0x8001
@@use_addr:
	lw		a1, BIN_size(s0)
	jal		checksum
	addiu	a2, r0, -1
	
	lw		a0, 4(sp)
	
	lw		v1, BIN_crc32(s0)
	nop
	bne		v0, v1, @@checksum_fail
	nop

	bne		a0, 1, @@skip_patch
	nop
	
	EnterCriticalSection			; Execute the patch binary
	
	lui		a0, 0x8001
	jalr	a0
	nop
	
	ExitCriticalSection

@@skip_patch:

	b		@@exit					; Return downloaded data size
	lw		v0, BIN_size(s0)
	
@@checksum_fail:

	move	s0, r0

@@message_loop:

	jal		prep_frame
	nop
	
	la		a0, text_csum_fail
	jal		DrawText_centered
	li		a1, 0x74
	
	la		a0, 0x00500020
	la		a1, 0x00500100
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
	blt		s0, 120, @@message_loop
	addi	s0, 1
	
	move	v0, r0
	
@@exit:

	lw		ra, 0(sp)
	addiu	sp, (8+BIN_len)
	jr		ra
	nop
	
	
do_loader_exec:

	la		v0, config
	lbu		v0, SET_bmode(v0)
	nop
	beq		v0, 1, @@unlock_trick
	nop
	beq		v0, 2, @@swap_trick
	nop
	b		@@no_trick
	nop
	
@@unlock_trick:

	jal		unlock_cdrom
	nop
	b		@@no_trick
	nop
	
@@swap_trick:

	jal		swap_trick
	nop
	
@@no_trick:

	EnterCriticalSection
	
	lui		a0, 0x1F80				; Disable all but CD and DMA interrupts
	sw		r0, IMASK(a0)
	sw		r0, ISTAT(a0)
	li		v0, 0x15
	sh		v0, SIO_CTRL_REG(a0)
	
	jal		HookDefaultInt
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
	
	; Turn off BIOS pad routine	
	jal		StopPAD
	nop
	
	lui		a0, 0x801F				; Set stack address
	ori		a0, 0xFFF0
	sw		a0, EXEC_saddr(s0)
	
	move	a0, s0					; Exec
	move	a1, r0
	jal		Exec
	move	a2, r0
	

checksum:

	; a0 - buffer
	; a1 - number of bytes
	; a2 - previous checksum or remainder
	
	addiu	sp, -8
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	
	addiu	a3, sp, -(4*256)
	
	jal		gen_table
	move	a0, a3
	
	lw		a0, 4(sp)
	nop
@@byte_loop:
	lbu		v0, 0(a0)
	addiu	a0, 1
	
	xor		v0, a2, v0				; crc = (crc>>8)^crcTable[(crc^byte)&0xff];
	andi	v0, 0xFF
	sll		v0, 2
	addu	v0, a3
	lw		v0, 0(v0)
	srl		a2, 8
	addiu	a1, -1
	bgtz	a1, @@byte_loop
	xor		a2, v0
	
	addiu	v0, r0, -1				; crc^0xFFFFFFFF
	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	xor		v0, a2, v0
	

gen_table:

	; a0 - table output
	
	move	t0, r0
@@calc_loop:
	move	v0, t0
	move	t1, r0
@@calc_loop_2:
	andi	v1, v0, 1
	beqz	v1, @@skip_xor
	srl		v0, 1
	lui		v1, 0xEDB8				; crcVal = (crcVal>>1)^0xEDB88320L;
	ori		v1, 0x8320
	xor		v0, v1
@@skip_xor:
	blt		t1, 7, @@calc_loop_2
	addiu	t1, 1

	sw		v0, 0(a0)
	addiu	a0, 4
	
	blt		t0, 255, @@calc_loop
	addiu	t0, 1

	jr		ra
	nop
