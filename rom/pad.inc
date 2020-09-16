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
	
	jal		_pad_wait				; 35-36 micro seconds as per 5k BIOS
	li		v0, 310                 
	
; May cause issues with third party adapters such as Brook wireless
;@@read_empty_fifo:					; Flush the RX FIFO just in case
;	lbu		v1, JOY_TXRX(t0)
;	lhu		v0, JOY_STAT(t0)
;	nop
;	andi	v0, 0x2
;	bnez	v0, @@read_empty_fifo
;	nop
	
	jal		_pad_exchange			; Send device check byte
	li		a0, 0x01

	andi	v1, v0, 0x100			; No pad if exchange timed out
	bnez	v1, @@no_device
	addiu	v0, r0 , 1
	
	sb		r0, 0(a1)				; first buffer byte always zeroed
	addiu	a1, 1

    jal     _pad_wait               ; 1st exchange needs 27microsec after ACK
    li      v0, 190                 ; (e.g. 7 as usual + an extra 20)
	
	jal		_pad_exchange			; Send second check byte
	li		a0, 0x42
	
    andi    a3, v0, 0xF             ; last 4 bits = num bytes to read
    sll     a3, a3, 0x1             ; double it (halfwords)
    addiu   a3, a3, 0x02            ; always read ( 1 + numbytes )

	bne	v0, 0x73, @@not_analogue	; if the pad's in analogue mode
	nop								; then pretend it isn't.
	li	v0,	0x41								


@@not_analogue:	

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
	
    addiu   a3, -1                  ; have we read as many bytes 
    beqz    a3, @@exit              ; as the pad wants us to?

	addiu	a2, -1                  ; have we reached the max buffer
	bgtz	a2, @@read_loop         ; length?
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

	addiu	sp, -4
	sw		ra, 0(sp)
	
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

    ; 7 microsec expected between TX/RX and ACK
    ; ( happens without any special timing )

	lhu		v1, JOY_CTRL(t0)
    nop
	ori		v1, 0x10            ; Ack
	sh		v1, JOY_CTRL(t0)

	jal		_pad_wait           ; Another 7 microsec expected
	li		v0, 60              ; after each ack

    ; the read has already happened,
    ; just return it from the buffer
    lbu		v0, JOY_TXRX(t0)

@@exit_exchg:

	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop


_pad_wait:
	addiu	v0, -1
	bgtz	v0, _pad_wait
	nop
	jr		ra
	nop
