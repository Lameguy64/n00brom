xptty_dev_info:
	.asciiz "n00bROM XPTTY device"
.align 4


TIMEOUT_VAL equ 5000


InstallTTY_xp:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	move	a0, r0				; Close stdin and stdout handles
	jal		rom_close
	addiu	sp, -4
	addiu	sp, 4
	addi	a0, r0, 1
	jal		rom_close
	addiu	sp, -4
	addiu	sp, 4
	
	la		a0, sio_dev_name	; Delete dummy TTY device
	jal		rom_DelDev
	addiu	sp, -4
	addiu	sp, 4
	
	la		a0, xptty_dcb		; Add n00bROM's serial TTY device
	jal		rom_AddDev
	addiu	sp, -4
	addiu	sp, 4
	
	la		a0, sio_dev_name	; Open stdin and stdout handles
	addiu	a1, r0, 2
	jal		rom_open
	addiu	sp, -8
	addiu	sp, 8
	la		a0, sio_dev_name
	addiu	a1, r0, 1
	jal		rom_open
	addiu	sp, -8
	addiu	sp, 8
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	

xptty_dev_init:
	jr		ra
	nop
	
	
xptty_dev_open:
	addi	v0, r0, 1
	sw		v0, FCB_diskid(a0)
	jr		ra
	move	v0, r0
	

xptty_dev_inout:

	addiu	sp, -12
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	sw		s0, 8(sp)
	
	move	s0, a0
	
	beq		a1, 1, @@read
	nop
	
@@send_loop:
	jal		xp_sendbyte_timeout_rom
	li		a0, 0x2F
	
	bltz	v0, @@exit
	nop
	
	lw		a1, FCB_taddr(s0)
	lw		a2, FCB_tlen(s0)
	lbu		a0, 0(a1)
	addi	a2, -1
	jal		xp_sendbyte_timeout_rom
	sw		a2, FCB_tlen(s0)
	
	bltz	v0, @@exit
	lw		a1, FCB_taddr(s0)
	lw		a2, FCB_tlen(s0)
	addiu	a1, 1
	bgtz	a2, @@send_loop
	sw		a1, FCB_taddr(s0)
	
	b		@@exit
	nop
	
@@read:
	lw		a1, FCB_taddr(a0)
	lw		a2, FCB_tlen(a0)
@@read_loop:
	jal		xp_readbyte_rom
	nop
	bne		v0, 0x2F, @@read_loop
	nop
	jal		xp_readbyte_rom
	nop
	sb		v0, 0(a1)
	addi	a2, -1
	bgtz	a2, @@read_loop
	addiu	a1, 1
	
@@exit:

	lw		a0, 4(sp)
	lw		ra, 0(sp)
	lw		v0, FCB_tlen(a0)
	lw		s0, 8(sp)
	jr		ra
	addiu	sp, 12
	
	
xptty_dev_ioctl:

	; FIOCSCAN is only supported
	bne		a1, (('f'<<8)|2), @@exit
	addiu	v0, r0, -1
	
	lui		a1, XP_IOBASE
	lbu		v0, XP_pcshake(a1)	; A high PCshake means xpcomms is trying to get
	nop							; the PS1's attention
	andi	v0, 0x1
	
	beqz	v0, @@exit
	addiu	v0, r0, -1
	
	move	v0, r0
	
@@exit:
	jr		ra
	nop
	
	
xptty_dcb:
	dw	sio_dev_name	; Device name
	dw	0x3				; Flags
	dw	0				; Size
	dw	xptty_dev_info	; Description
	dw	xptty_dev_init	; init
	dw	xptty_dev_open	; open
	dw	xptty_dev_inout	; inout
	dw	0				; close
	dw	xptty_dev_ioctl	; ioctl
	dw	0				; read
	dw	0				; write
	dw	0				; erase
	dw	0				; undelete
	dw	0				; firstfile
	dw	0				; nextfile
	dw	0				; format
	dw	0				; chdir
	dw	0				; rename
	dw	0				; remove
	dw	0				; testdevice



xp_readbyte_rom:
	lui		t0, XP_IOBASE			

@@shake_wait:
	lbu		v1, XP_pcshake(t0)		; Wait until PC -> XP handshake is high
	nop
	andi	v1, 0x1
	beqz	v1, @@shake_wait
	nop

	lbu		v0, XP_data(t0)			; Read data in port
	
	li		v1, 0x8					; Set XP -> PC ack high
	sb		v1, XP_latch(t0)
	
@@ack_wait:							; Wait until PC -> XP handshake is low
	lbu		v1, XP_pcshake(t0)
	nop
	andi	v1, 0x1
	bnez	v1, @@ack_wait
	nop
	
	sb		r0, XP_latch(t0)		; Set XP -> PC ack low
	
	jr		ra
	nop
	
	
xp_sendbyte_rom:

	addiu	sp, -4
	andi	a0, 0xFF
	sw		a0, 0(sp)
	
	lui		a1, XP_IOBASE
	move	a2, r0

@@retry:
	
@@wait_pcack:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is low
	nop
	andi	v1, 0x1
	bnez	v1, @@wait_pcack
	nop
	
	andi	v0, a0, 0x7				; Set first three bits
	ori		v0, 0x8					; Brings ACK high
	sb		v0, XP_latch(a1)
	
	andi	v0, a0, 1				; Compute parity
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
		
@@shake_wait_1:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is high
	nop
	andi	v1, 0x1
	beqz	v1, @@shake_wait_1
	nop

	andi	v0, a0, 0x7				; Set next three bits
	sb		v0, XP_latch(a1)		; Leave ACK low
	
	andi	v0, a0, 1				; Compute parity
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	
@@shake_wait_2:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is low
	nop
	andi	v1, 0x1
	bnez	v1, @@shake_wait_2
	nop	
	
	andi	v0, a0, 0x3				; Set the last two bits

	andi	v1, a0, 1				; Compute parity
	xor		a2, v1
	srl		a0, 1
	andi	v1, a0, 1
	xor		a2, v1
	andi	a2, 1
	sll		a2, 2
	or		v0, a2
	
	ori		v0, 0x8					; Brings ACK high
	sb		v0, XP_latch(a1)
	nop
	
@@shake_wait_3:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is high
	nop
	andi	v1, 0x1
	beqz	v1, @@shake_wait_3
	nop	
	
	sb		r0, XP_latch(a1)		; Clear ACK

	lbu		v0, XP_data(a1)			; Read data to check if transfer is okay

@@shake_wait_4:
	lbu		v1, XP_pcshake(a1)	; Wait until PC -> XP handshake is low
	nop
	andi	v1, 0x1
	bnez	v1, @@shake_wait_4
	nop	
	
	bnez	v0, @@retry
	lw		a0, 0(sp)
	
	jr		ra
	addiu	sp, 4
	

xp_sendbyte_timeout_rom:

	addiu	sp, -4
	andi	a0, 0xFF
	sw		a0, 0(sp)
	
	lui		a1, XP_IOBASE
	move	a2, r0

@@retry:
	
	move	t0, r0
@@wait_pcack:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is low
	addiu	t0, 1
	bgt		t0, TIMEOUT_VAL, @@timeout
	andi	v1, 0x1
	bnez	v1, @@wait_pcack
	nop
	
	andi	v0, a0, 0x7				; Set first three bits
	ori		v0, 0x8					; Brings ACK high
	sb		v0, XP_latch(a1)
	
	andi	v0, a0, 1				; Compute parity
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	
	move	t0, r0
@@shake_wait_1:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is high
	nop
	addiu	t0, 1
	bgt		t0, TIMEOUT_VAL, @@timeout
	andi	v1, 0x1
	beqz	v1, @@shake_wait_1
	nop

	andi	v0, a0, 0x7				; Set next three bits
	sb		v0, XP_latch(a1)		; Leave ACK low
	
	andi	v0, a0, 1				; Compute parity
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	andi	v0, a0, 1
	xor		a2, v0
	srl		a0, 1
	
	move	t0, r0
@@shake_wait_2:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is low
	addiu	t0, 1
	bgt		t0, TIMEOUT_VAL, @@timeout
	andi	v1, 0x1
	bnez	v1, @@shake_wait_2
	nop	
	
	andi	v0, a0, 0x3				; Set the last two bits

	andi	v1, a0, 1				; Compute parity
	xor		a2, v1
	srl		a0, 1
	andi	v1, a0, 1
	xor		a2, v1
	andi	a2, 1
	sll		a2, 2
	or		v0, a2
	
	ori		v0, 0x8					; Brings ACK high
	sb		v0, XP_latch(a1)
	nop
	
	move	t0, r0
@@shake_wait_3:
	lbu		v1, XP_pcshake(a1)		; Wait until PC -> XP handshake is high
	addiu	t0, 1
	bgt		t0, TIMEOUT_VAL, @@timeout
	andi	v1, 0x1
	beqz	v1, @@shake_wait_3
	nop	
	
	sb		r0, XP_latch(a1)		; Clear ACK

	lbu		v0, XP_data(a1)			; Read data to check if transfer is okay

	move	t0, r0
@@shake_wait_4:
	lbu		v1, XP_pcshake(a1)	; Wait until PC -> XP handshake is low
	addiu	t0, 1
	bgt		t0, TIMEOUT_VAL, @@timeout
	andi	v1, 0x1
	bnez	v1, @@shake_wait_4
	nop
	
	bnez	v0, @@retry
	lw		a0, 0(sp)

	b		@@exit
	move	v0, r0

@@timeout:

	addiu	v0, r0, -1
	
@@exit:
	
	jr		ra
	addiu	sp, 4
