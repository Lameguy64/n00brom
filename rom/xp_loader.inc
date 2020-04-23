xp_readbyte:
	lui		a0, XP_IOBASE			

@@shake_wait:
	lbu		v1, XP_pcshake(a0)		; Wait until PC -> XP handshake is high
	nop
	andi	v1, 0x1
	beqz	v1, @@shake_wait
	nop

	lbu		v0, XP_data(a0)			; Read data in port
	
	li		v1, 0x8					; Set XP -> PC ack high
	sb		v1, XP_latch(a0)
	
@@ack_wait:							; Wait until PC -> XP handshake is low
	lbu		v1, XP_pcshake(a0)
	nop
	andi	v1, 0x1
	bnez	v1, @@ack_wait
	nop
	
	sb		r0, XP_latch(a0)		; Set XP -> PC ack low
	
	jr		ra
	nop
	
	
xp_readbytes:
	
	addiu	sp, -8
	sw		ra, 0(sp)
	
@@read_loop:
	sw		a0, 4(sp)				; Save destination address
	jal		xp_readbyte				; Read a byte
	nop
	lw		a0, 4(sp)				; Restore destination address
	addiu	a1, -1					; Decrement bytes to read
	sb		v0, 0(a0)				; Store read byte to destination address
	bgtz	a1, @@read_loop			; Loop
	addiu	a0, 1					; Increment for next byte
	
	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	nop
	
	
xplorer_loader:

	addiu	sp, -4
	sw		ra, 0(sp)
	
	subiu	sp, EXEC_len			; Allocate stack for the PS-EXE params
	move	s0, sp
	
	move	a0, s0					; Receive PS-EXE parameters
	jal		xp_readbytes
	li		a1, EXEC_len
	
	la		a0, text_download
	jal		sort_status
	nop
	
	lw		a0, EXEC_taddr(s0)		; Download EXE body
	jal		xp_readbytes
	lw		a1, EXEC_tsize(s0)
	
	lw		a0, EXEC_taddr(s0)		; Checksum the executable
	lw		a1, EXEC_tsize(s0)
	jal		checksum
	addiu	a2, r0, -1
	
	lw		v1, EXEC_crc32(s0)		; Test checksums
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
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop


xplorer_binloader:
	
	addiu	sp, -4
	sw		ra, 0(sp)
	
	subiu	sp, BIN_len			; Allocate stack for the binary download params
	move	s0, sp
	
	move	a0, s0				; Receive binary parameters
	jal		xp_readbytes
	li		a1, BIN_len
	
	la		a0, text_download_bin
	jal		sort_status
	nop
	
	lw		a0, BIN_addr(s0)	; Download binary file data
	jal		xp_readbytes
	lw		a1, BIN_size(s0)
	
	lw		a0, BIN_addr(s0)	; Checksum the executable
	lw		a1, BIN_size(s0)
	jal		checksum
	addiu	a2, r0, -1
	
	lw		v1, BIN_crc32(s0)	; Test checksum
	nop
	beq		v0, v1, @@exit
	nop

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
	
@@exit:
	
	addiu	sp, BIN_len
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
