GP0			equ 0x1810
GP1			equ 0x1814
D2_MADR		equ 0x10A0
D2_BCR		equ 0x10A4
D2_CHCR		equ 0x10A8


.macro tpage, reg, bpp, dtd, x, y
	la		reg, (((x>>6)&0xF)|((y>>8)<<4)|((bpp&0x3)<<7)|((dtd&0x1)<<10)|(1<<9))
.endmacro


InitGPU:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	lui		t0, 0x1F80
	sw		r0, GP1(t0)			; Reset
	
	lui		v0, 0x06C5			; Set the horizontal display range
	ori		v0, 0x8258
	sw		v0, GP1(t0)
	
	la		v0, config
	lbu		v0, SET_vmode(v0)
	nop
	beq		v0, 2, @@is_pal
	nop
	bnez	v0, @@is_ntsc
	nop
	
	li		v0, 0xBFC7FF52
	lbu		v0, 0(v0)
	nop
	bne		v0, 'E', @@is_ntsc	; Must do a !='E' check or else a Japanese PSX will treat it as PAL
	nop

@@is_pal:
	
	li		v0, 0x07049425		; Set the video mode for PAL
	sw		v0, GP1(t0)
	li		v0, 0x08000009
	sw		v0, GP1(t0)
	
	b		@@done_set
	nop
	
@@is_ntsc:

	li		v0, 0x07040010		; Set the video mode for NTSC
	sw		v0, GP1(t0)
	li		v0, 0x08000001
	sw		v0, GP1(t0)
	nop

@@done_set:

	jal		WaitGPU
	nop
	
	tpage	v0, 0, 1, 960, 0	; Configure the drawing environment
	lui		v1, 0xE100
	or		v0, v1
	sw		v0, GP0(t0)
	
	jal		WaitGPU
	nop
	li		v0, 0xE3000000		; Draw clip start (0, 256)
	sw		v0, GP0(t0)
	
	jal		WaitGPU
	nop
	li		v0, 0xE4043D3F		; Draw clip end (320, 512)
	sw		v0, GP0(t0)
	
	jal		WaitGPU
	nop
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop


; Upload data to SGRAM/VRAM via GPU I/O.
;
; Advantage: Source pixel data can be from ROM
;
; Parameters:
;	a0 - Address to image data.
;	a1 - VRAM destination address ($yyyyxxxx)
;	a2 - VRAM destination size ($hhhhwwww)
;	a3 - Number of bytes to send (must be a multiple of 4 bytes)
;
LoadImage_IO:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	jal		WaitGPU
	lui		t0, 0x1F80
	
	lui		v0, 0x0400		; GP1 set DMA direction command (to off)
	sw		v0, GP1(t0)
	lui		v0, 0x0100		; GP1 Reset command buffer command
	lui		v1, 0xA000		; GP0 Copy rectangle command
	sw		v0, GP1(t0)		
	sw		v1, GP0(t0)
	sw		a1, GP0(t0)		; Destination offset
	sw		a2, GP0(t0)		; Width + Height

@@loop:						; Write the image data into the GP0 port

	lw		v0, 0(a0)
	addi	a3, -4
	sw		v0, GP0(t0)
	addiu	a0, 4
	bgtz	a3, @@loop
	nop
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop


; Waits for the GPU to become available to accept a command
WaitGPU:
	lui		v1, 0x1F80
	lw		v0, GP1(v1)
	;lui		v1, 0x1000
	;and		v0, v1
	nop
	srl		v0, 26
	andi	v0, 1
	beqz	v0, WaitGPU
	nop
	jr		ra
	nop

	
SetDrawPage:

	; a0 - draw page
	
	; Destroys: v0, v1, a1
	
	addiu	sp, -4
	sw		ra, 0(sp)

	lui		a1, 0x1F80
		
	bnez	a0, @@do_set
	li		a2, 256
	
	move	a2, r0
	
@@do_set:

	sll		v0, a2, 10			; Set display area address
	lui		v1, 0x0500
	or		v0, v1
	sw		v0, GP1(a1)
	
	lui		v0, 0x0300			; Enable display
	sw		v0, GP1(a1)
	
	beqz	a0, @@do_set_2
	li		a2, 256
	
	move	a2, r0
	
@@do_set_2:
	
	jal		WaitGPU
	lui		a1, 0x1F80
	
	sll		v1, a2, 11			; Draw offset
	lui		v0, 0xE500
	or		v0, v1
	sw		v0, GP0(a1)
	
	jal		WaitGPU
	nop
	
	sll		v1, a2, 10			; Draw clip start
	lui		v0, 0xE300
	or		v0, v1
	sw		v0, GP0(a1)
	
	jal		WaitGPU
	nop
	
	addiu	a2, 255				; Draw clip end
	sll		v1, a2, 10
	lui		v0, 0xE400
	or		v0, v1
	ori		v0, 0x13F
	sw		v0, GP0(a1)
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
DrawSync:
	lui		a0, 0x1F80

	lw      v0, GP1(a0)
	nop
	srl		v0, 29
	andi	v0, 0x3
	
	beqz	v0, @simple_wait
	nop
	
@dma_wait:

	lw		v0, D2_CHCR(a0)
	nop
	srl		v0, 24
	andi	v0, 0x1
	bnez	v0, @dma_wait
	nop

@@gpu_wait:

	lw      v0, GP1(a0)
	nop
	srl		v0, 26
	andi	v0, 0x5
	bne		v0, 5, @@gpu_wait
	nop
	
	b		@exit
	nop
	
@simple_wait:					; Wait for GPU to be ready for next DMA

	lw		v0, GP1(a0)
	nop
	srl		v0, 28
	andi	v0, 0x1
	beqz	v0, @simple_wait
	nop

@exit:

	jr		ra
	nop
	

AddPrim:

	lw		v0, 0(a0)			; Load OT entry
	lw		v1, 0(a1)			; Set packet length value (in words)
	lui		at, 0x00ff
	ori		at, 0xffff
	and		v0, at				; Mask off the upper 8 bits of OT entry
	or		v1, v0				; OR values together
	sw		v1, 0(a1)			; Store new address to primitive tag
	lw		v0, 0(a0)			; Load OT entry
	and		a1, at				; Mask off the upper 8 bits of primitive tag
	lui		at, 0xff00
	and		v0, at				; Mask off the first 24 bits of OT entry
	or		v0, a1				; OR values together

	jr		ra
	sw		v0, 0(a0)			; Store result to OT
	
	
DrawOTag:

	lui		a3, 0x1f80			; I/O segment base

	lui		v0, 0x0400			; Set DMA direction to CPUtoGPU
	ori		v0, 0x2
	sw		v0, GP1(a3)
	
@@gpu_wait:						; Wait for GPU to be ready for commands & DMA

	lw		v0, GP1(a3)
	nop
	srl		v0, 26
	andi	v0, 1
	beqz	v0, @@gpu_wait
	nop

	sw		a0, D2_MADR(a3)		; Set DMA base address to specified OT
	sw		r0, D2_BCR(a3)

	lui		v0, 0x0100			; Begin OT transfer!
	ori		v0, 0x0401
	sw		v0, D2_CHCR(a3)

	jr		ra
	nop
