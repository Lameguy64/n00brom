; 04-23-2020 Lameguy64:
; There's issues reading certain controllers on PAL units with this pad read
; routine. Can't debug myself as I don't have a PAL unit to test on. Some
; controllers read properly however, maybe needs more delays elsewhere?

JOY_TXRX	equ		0x1040
JOY_STAT	equ		0x1044
JOY_MODE	equ		0x1048
JOY_CTRL	equ		0x104A
JOY_BAUD	equ		0x104E

PadInit:

	addiu	sp, -4
	sw		ra, 0(sp)
	
	lui		t0, 0x1F80				; Interface setup
	
	li		v0, 0x40				; Interface reset
	sh		v0, JOY_CTRL(t0)
	li		v0, 0x88				; 250kHz clock rate
	sh		v0, JOY_BAUD(t0)
	li		v0, 0x0d				; 8-bit, no parity, x1 multiplier
	sh		v0, JOY_MODE(t0)
	li		v0, 0x1003				; JOY1, TX enabled
	sh		v0, JOY_CTRL(t0)

	jal		_pad_wait
	li		v0, 1000
	
@@empty_fifo:						; Empty RX fifo
	lbu		v1, JOY_TXRX(t0)
	lhu		v0, JOY_STAT(t0)
	nop
	andi	v0, 0x2
	bnez	v0, @@empty_fifo
	nop
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
PadRead:

	; a0 - port number
	; a1 - device data buffer
	; a2 - data max length
	
	addiu	sp, -4
	sw		ra, 0(sp)
	
	lui		t0, 0x1F80
	
	li		v0, 0x1003				; TX Enable, Joypad port select
	andi	a0, 1
	sll		a0, 13
	or		v0, a0					; Select port 2 if a0 is 1
	
	sh		v0, JOY_CTRL(t0)		; Set to Joypad control interface
	
	jal		_pad_wait				; Delay for analog pads (needs testing)
	li		v0, 500
	
@@read_empty_fifo:					; Flush the RX FIFO just in case
	lbu		v1, JOY_TXRX(t0)
	lhu		v0, JOY_STAT(t0)
	nop
	andi	v0, 0x2
	bnez	v0, @@read_empty_fifo
	nop
	
	jal		_pad_exchange			; Send device check byte
	li		a0, 0x01

	andi	v1, v0, 0x100			; No pad if exchange timed out
	bnez	v1, @@no_device
	addiu	v0, r0 , 1
	
	sb		r0, 0(a1)
	addiu	a1, 1
	
	jal		_pad_exchange			; Send second check byte
	li		a0, 0x42
	
	sb		v0, 0(a1)
	addiu	a1, 1
	addiu	a2, -2
	
	jal		_pad_exchange			; Send 0 for pads, 1 for multitap
	move	a0, r0					; Read is usually 0x5A
	
@@read_loop:						; Read until buffer full, or no more data
	jal		_pad_exchange			; when ACK is no longer triggered
	move	a0, r0
	sb		v0, 0(a1)
	
	andi	v0, 0x100
	bnez	v0, @@exit
	nop
	
	addiu	a2, -1
	bgtz	a2, @@read_loop
	addiu	a1, 1
	
	b		@@exit
	nop
	
@@no_device:
	
	addiu	v0, r0 , -1
	sb		v0, 0(a1)
	
@@exit:
	
	sh		r0 , JOY_CTRL(t0)
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
_pad_exchange:
	
	lui		t0, 0x1F80
	
	sb		a0, JOY_TXRX(t0)
	nop
@@send_wait:
	lhu		v0, JOY_STAT(t0)
	nop
	andi	v0, 0x4
	beqz	v0, @@send_wait
	nop
	
	move	v1, r0
@@wait_rx:
	bgt		v1, 100, @@timeout
	lhu		v0, JOY_STAT(t0)
	nop
	andi	v0, 0x202
	bne		v0, 0x202, @@wait_rx
	addiu	v1, 1
	
	b		@@done
	nop
	
@@timeout:

	lbu		v0, JOY_TXRX(t0)
	nop
	b		@@exit_exchg
	ori		v0, 0x100

@@done:

	lhu		v1, JOY_CTRL(t0)
	lbu		v0, JOY_TXRX(t0)
	ori		v1, 0x10
	sh		v1, JOY_CTRL(t0)
	
@@exit_exchg:

	jr		ra
	nop


_pad_wait:
	addiu	v0, -1
	bgtz	v0, _pad_wait
	nop
	jr		ra
	nop
