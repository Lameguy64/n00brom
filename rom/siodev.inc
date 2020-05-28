; Serial routines and TTY device hook

sioInit_rom:
	EnterCriticalSection
	
	lui		a0, 0x1F80
	
	li		v0, 0x40
	sh		v0, SIO_CTRL_REG(a0)
	li		v0, 0x4E
	sh		v0, SIO_MODE_REG(a0)
	li		v0, 0x12
	sh		v0, SIO_BAUD_REG(a0)
	li		v0, 0x815
	sh		v0, SIO_CTRL_REG(a0)
	
	lhu		v0, IMASK(a0)
	nop
	ori		v0, 0x100
	sh		v0, IMASK(a0)
	
	ExitCriticalSection

	jr		ra
	nop
	
sioReadByte_rom:
	; Read a byte from the serial port.
	; Returns: v0 - byte read.
	la		v0, SIO_STAT_REG_A	; Read value from serial
	lw		v0, 0(v0)
	nop
	andi	v0, 0x2
	beqz	v0, sioReadByte_rom
	nop
	la		v0, SIO_TXRX_REG_A
	lbu		v0, 0(v0)
	jr		ra
	nop
	
sioSendByte_rom:
	; Send a byte to the serial port.
	; Input: a0 - byte to send to serial.
	; Returns: none
	addiu	sp, -4
	sw		v0, 0(sp)
@@tx_ready:						; Wait for TX to become ready
	la		v0, SIO_STAT_REG_A
	lhu		v0, 0(v0)
	nop	
	andi	v0, 0x1
	beqz	v0, @@tx_ready
	nop
	la		v0, SIO_TXRX_REG_A	; Send byte
	sb		a0, 0(v0)
@@tx_done:						; Wait for TX to finish
	la		v0, SIO_STAT_REG_A
	lhu		v0, 0(v0)
	nop
	andi	v0, 0x4
	beqz	v0, @@tx_done
	nop
	lw		v0, 0(sp)			; Restore stack and return
	jr		ra
	addiu	sp, 4


InstallTTY_sio:
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
	
	la		a0, siotty_dcb		; Add n00bROM's serial TTY device
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
	
	;jal		PrintDev
	;nop
	
	lw		ra, 0(sp)
	addiu	sp, 4
sio_dev_dummy:
	jr		ra
	nop
		
	
sio_dev_open:

	addi	v0, r0, 1
	sw		v0, FCB_diskid(a0)
	jr		ra
	move	v0, r0
	

sio_dev_inout:

	addiu	sp, -8
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	
	beq		a1, 1, @@read
	nop
	
	lw		a1, FCB_taddr(a0)
	lw		a2, FCB_tlen(a0)
@@send_loop:
	lbu		a0, 0(a1)
	jal		sioSendByte_rom
	addi	a2, -1
	bgtz	a2, @@send_loop
	addiu	a1, 1
	lw		a0, 4(sp)
	lw		ra, 0(sp)
	lw		v0, FCB_tlen(a0)
	jr		ra
	addiu	sp, 8
	
@@read:
	lw		a1, FCB_taddr(a0)
	lw		a2, FCB_tlen(a0)
@@read_loop:
	jal		sioReadByte_rom
	nop
	sb		v0, 0(a1)
	addi	a2, -1
	bgtz	a2, @@read_loop
	addiu	a1, 1
	
	lw		ra, 0(sp)
	lw		v0, FCB_tlen(a0)
	jr		ra
	addiu	sp, 8
	
	
sio_dev_close:
	jr		ra
	move	v0, r0
	
	
sio_dev_ioctl:

	; FIOCSCAN is only supported
	bne		a1, (('f'<<8)|2), @@exit
	addiu	v0, r0, -1
	
	lui		a1, 0x1F80
	lbu		v0, SIO_STAT_REG(a1)
	nop
	andi	v0, 0x2
	
	beqz	v0, @@exit
	addiu	v0, r0, -1
	
	move	v0, r0
	
@@exit:
	jr		ra
	nop
	
	
rom_open:
	addiu   t2, r0, 0xB0
	jr      t2
	addiu   t1, r0, 0x32
	
rom_close:
	addiu   t2, r0, 0xB0
	jr      t2
	addiu   t1, r0, 0x36
	
rom_AddDev:
	addiu	t2, r0, 0xB0
	jr		t2
	addiu	t1, r0, 0x47
	
rom_DelDev:
	addiu	t2, r0, 0xB0
	jr		t2
	addiu	t1, r0, 0x48
	

; Serial TTY IOB	
siotty_dcb:
	dw	sio_dev_name	; Device name
	dw	0x3				; Flags
	dw	0				; Size
	dw	sio_dev_info	; Description
	dw	sio_dev_dummy	; init
	dw	sio_dev_open	; open
	dw	sio_dev_inout	; inout
	dw	sio_dev_dummy	; close
	dw	sio_dev_ioctl	; ioctl
	dw	sio_dev_dummy	; read
	dw	sio_dev_dummy	; write
	dw	sio_dev_dummy	; erase
	dw	sio_dev_dummy	; undelete
	dw	sio_dev_dummy	; firstfile
	dw	sio_dev_dummy	; nextfile
	dw	sio_dev_dummy	; format
	dw	sio_dev_dummy	; chdir
	dw	sio_dev_dummy	; rename
	dw	sio_dev_dummy	; remove
	dw	sio_dev_dummy	; testdevice

sio_dev_name:
	.asciiz "tty"
sio_dev_info:
	.asciiz "n00bROM SIOTTY device"
.align 4
