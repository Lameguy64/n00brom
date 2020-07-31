; Exception handler, for trapping unhandled exceptions

PCB_addr	equ	0x108

TCB_status	equ	0
TCB_reserve	equ	4
TCB_r0		equ	8
TCB_at		equ	12
TCB_v0		equ	16
TCB_v1		equ	20
TCB_a0		equ	24
TCB_a1		equ	28
TCB_a2		equ	32
TCB_a3		equ	36
TCB_t0		equ	40
TCB_t1		equ	44
TCB_t2		equ	48
TCB_t3		equ	52
TCB_t4		equ	56
TCB_t5		equ	60
TCB_t6		equ	64
TCB_t7		equ	68
TCB_s0		equ	72
TCB_s1		equ	76
TCB_s2		equ	80
TCB_s3		equ	84
TCB_s4		equ	88
TCB_s5		equ	92
TCB_s6		equ	96
TCB_s7		equ	100
TCB_t8		equ	104
TCB_t9		equ	108
TCB_k0		equ	112
TCB_k1		equ	116
TCB_gp		equ	120
TCB_sp		equ	124
TCB_fp		equ	128
TCB_ra		equ	132
TCB_epc		equ	136
TCB_hi		equ	140
TCB_lo		equ	144
TCB_sr		equ	148
TCB_cause	equ	152


install_exception:
	
	li		v0, 0x200				; Hooks SystemErrorUnresolvedException()
	li		v1, 0x40
	sll		v1, 2
	addu	v0, v1
	la		v1, syserr_unresolved
	sw		v1, 0(v0)
	
	jr		ra
	nop

	
syserr_unresolved:					; Called by BIOS kernel
	
	mfc0	v0, CAUSE				; Get cop0 CAUSE and EPC values
	mfc0	v1, EPC
	
	srl		v0, 2					; Determine if the cause is a break opcode
	andi	v0, 0xF
	bne		v0, 0x9, @@not_break
	nop
	
	lw		v0, 0(v1)				; Extract the break opcode's value
	lw		v1, PCB_addr(r0)
	srl		v0, 6
	andi	v0, 0xFFFF
	bne		v0, 1024, @@not_break	; libsn pollhost() break opcode value
	nop
	
	lw		v1, 0(v1)				; Adjust EPC to skip break opcode in TCB
	nop
	lw		v0, TCB_epc(v1)
	nop
	addiu	v0, 4
	sw		v0, TCB_epc(v1)
	
	addiu	t2, r0, 0xB0			; ReturnFromException()
	jr		t2
	addiu	t1, r0, 0x17
	
@@not_break:						; Real, unhandled exception

	addiu	sp, -32					; Set workspace area address
	move	gp, sp
	
	jal		ex_InitGPU
	nop
	
	la		a0, ex_font_image
	jal		ex_LoadTIM
	move	a1, r0
	
	lui		v1, 0x1f80
	lui		v0, 0x027f			; FillVRAM command
	sw		v0, GP0(v1)
	sw		r0, GP0(v1)
	lui		v0, 0x0100			; y = 256
	ori		v0, 0x0140			; x = 320
	sw		v0, GP0(v1)
	jal		ex_WaitGPU
	nop
	
	jal		ex_ShowDisplay
	nop
	
	mfc0	v1, CAUSE
	nop
	srl		v1, 2
	andi	v1, 0x1f
	la		v0,	crash_msg
	sll		v1, 2
	addu	v0, v1
	lw		a0, 0(v0)
	
	la		a1, 0x00100008
	jal		ex_drawtext
	nop
	
	la		a0, label_epc
	la		a1, 0x00200008
	jal		ex_drawtext
	nop
	mfc0	a0, EPC
	nop
	jal		ex_reg2hex
	move	a1, gp
	move	a0, gp
	jal		ex_drawtext
	move	a1, v0
	addiu	v0, 8
	
	la		a0, label_badaddr
	jal		ex_drawtext
	move	a1, v0
	mfc0	a0, BADVADDR
	nop
	jal		ex_reg2hex
	move	a1, gp
	move	a0, gp
	jal		ex_drawtext
	move	a1, v0
	
	la		a0, label_op
	la		a1, 0x00280008
	jal		ex_drawtext
	nop
	mfc0	t0, EPC
	addiu	a0, r0, -1
	andi	t1, t0, 0x3
	bnez	t1, @@pc_notaligned
	nop
	lw		a0, 0(t0)
@@pc_notaligned:
	nop
	jal		ex_reg2hex
	move	a1, gp
	move	a0, gp
	jal		ex_drawtext
	move	a1, v0
	
	la		a0, label_cause
	addiu	a1, v0, 8
	jal		ex_drawtext
	nop
	mfc0	a0, CAUSE
	nop
	jal		ex_reg2hex
	move	a1, gp
	move	a0, gp
	move	a1, v0
	jal		ex_drawtext
	nop
	
	lw		s1, PCB_addr(r0)		; Print out CPU registers
	la		s0, reg_labels
	lw		s1, 0(s1)
	li		s3, 0x0008
	li		s5, 3
	addiu	s1, TCB_at
	
@@reg_loop_col:
	
	li		s2, 0x0038
	li		s4, 11
	
@@reg_loop_row:
	
	bne		s5, 1, @@skip_change	; To make the register pointer skip EPC
	nop								; before drawing the hi/lo registers
	bne		s4, 2, @@skip_change
	nop
	addiu	s1, 4
@@skip_change:
	
	sll		a1, s2, 16
	or		a1, s3
	
	move	a0, s0
	jal		ex_drawtext
	addiu	s0, 4
	lw		a0, 0(s1)
	addiu	s1, 4
	jal		ex_reg2hex
	move	a1, gp
	
	move	a0, gp
	move	a1, v0
	jal		ex_drawtext
	nop
	
	addi	s4, -1
	bgtz	s4, @@reg_loop_row
	addi	s2, 8
	
	addi	s5, -1
	bgtz	s5, @@reg_loop_col
	addi	s3, 0x60
	
	la		a0, label_stack
	la		a1, 0x00980008
	jal		ex_drawtext
	nop

	lw		s0, PCB_addr(r0)		; Print the stack contents
	li		s1, 0x00A0
	lw		s0, 0(s0)
	li		s3, 8
	lw		s0, TCB_sp(s0)
	
@@stack_row_loop:

	li		s2, 4
	sll		v0, s1, 16
	ori		v0, 8
	
@@stack_col_loop:
	
	lw		a0, 0(s0)
	addiu	s0, 4
	jal		ex_reg2hex
	move	a1, gp
	move	a0, gp
	jal		ex_drawtext
	move	a1, v0
	
	addiu	s2, -1
	bgtz	s2, @@stack_col_loop
	addiu	v0, 8

	addiu	s3, -1
	bgtz	s3, @@stack_row_loop
	addiu	s1, 8
	
@@end:
	b		@@end
	nop
	
	
ex_drawtext:
	addiu	sp, -4
	sw		ra, 0(sp)

@@draw_loop:

	lbu		v1, 0(a0)
	nop
	beqz	v1, @@draw_end
	nop
	addi	v1, -33
	bltz	v1, @@skip
	addiu	a0, 1
	
	li		v0, 16
	div		v1, v0
	
	lui		a2, 0x1f80
	li		v0, 0x74808080		; Primitive packet
	sw		v0, GP0(a2)
	sw		a1, GP0(a2)			; Screen position
	
	mfhi	v0					; X tile coordinate
	nop
	sll		v0, 3
	mflo	v1
	nop
	sll		v1, 11
	or		v0, v1
	lui		v1, 0x0814
	or		v0, v1
	sw		v0, GP0(a2)
	
	jal		ex_WaitGPU
	nop
	
@@skip:

	b		@@draw_loop
	addi	a1, 8
	
@@draw_end:
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	move	v0, a1
		

ex_reg2hex:
	; a0 - value
	; a1 - buffer output
	
	la		a2, hex_tbl
	move	t0, r0
	addiu	a1, 7
	sb		r0, 1(a1)
	
@@hex_loop:

	andi	a3, a0, 0xF
	addu	v1, a2, a3
	lbu		v1, 0(v1)
	srl		a0, 4
	sb		v1, 0(a1)
	addiu	a1, -1
	blt		t0, 7, @@hex_loop
	addi	t0, 1
	
	jr		ra
	nop
	

ex_LoadTIM:

	addiu	sp, -8
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	
	lw		v0, TIM_FLAG(a0)
	
	addiu	a0, TIM_DATA		; Get information about the pixel data block
	
	andi	v0, 0x8
	beqz	v0, @@no_clut
	nop
	
	lw		v0, TIMBLK_DX(a0)
	lw		a2, TIMBLK_W(a0)
	lw		a3, TIMBLK_SIZE(a0)
	addiu	a0, TIMBLK_SLEN
	
	beqz	a1, @@no_store_clut
	nop
	sw		v0, 0(a1)			; Save CLUT value
@@no_store_clut:
	move	a1, v0
	
	jal		ex_LoadImage_IO		; Upload to SGRAM/VRAM
	subi	a3, TIMBLK_SLEN

	lw		a0, 4(sp)
	nop
	addiu	a0, TIM_DATA
	lw		a3, TIMBLK_SIZE(a0)
	nop
	addu	a0, a3
	
@@no_clut:
	
	lw		a1, TIMBLK_DX(a0)
	lw		a2, TIMBLK_W(a0)
	lw		a3, TIMBLK_SIZE(a0)
	addiu	a0, TIMBLK_SLEN
	
	jal		ex_LoadImage_IO		; Upload to SGRAM/VRAM
	subi	a3, TIMBLK_SLEN

	lw		ra, 0(sp)
	lw		a0, 4(sp)
	jr		ra
	addiu	sp, 8
	

ex_InitGPU:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	lui		t0, 0x1F80
	sw		r0, GP1(t0)			; Reset
	
	jal		ex_WaitGPU
	nop
	
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

	jal		ex_WaitGPU				; Configure the drawing environment
	nop
	;li		v0, 0xE1000605		; Draw mode, texture page = (4bit, 320,0)
	
	tpage	v0, 0, 1, 960, 0
	lui		v1, 0xE100
	or		v0, v1
	sw		v0, GP0(t0)
	
	jal		ex_WaitGPU
	nop
	li		v0, 0xE3000000		; Draw clip start (0, 0)
	sw		v0, GP0(t0)
	
	jal		ex_WaitGPU
	nop
	li		v0, 0xE4043D3F		; Draw clip end (320, 256)
	sw		v0, GP0(t0)
	
	jal		ex_WaitGPU
	nop
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop


ex_ShowDisplay:
	li		v0, 0x03000000
	jr		ra
	sw		v0, GP1(t0)			; enable the display


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
ex_LoadImage_IO:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	jal		ex_WaitGPU
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
ex_WaitGPU:
	lui		v1, 0x1F80
	lw		v0, GP1(v1)
	;lui		v1, 0x1000
	;and		v0, v1
	nop
	srl		v0, 26
	andi	v0, 1
	beqz	v0, ex_WaitGPU
	nop
	jr		ra
	nop

	
ex_SetDrawPage:

	; a0 - draw page
	
	; Destroys: v0, v1, a1
	
	addiu	sp, -4
	sw		ra, 0(sp)

	lui		a1, 0x1F80
		
	bnez	a0, @@do_set
	move	a2, r0
	
	li		a2, 256
	
@@do_set:

	sll		v0, a2, 10
	lui		v1, 0x0500
	or		v0, v1
	sw		v0, GP1(a1)
	
	lui		v0, 0x0300			; Enable display
	sw		v0, GP1(a1)
	
	beqz	a0, @@do_set_2
	move	a2, r0
	
	li		a2, 256
	
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
	
	
ex_DrawSync:
	lui		a0, 0x1F80

	lw      v0, GP1(a0)
	nop
	srl		v0, 29
	andi	v0, 0x3
	
	beqz	v0, @@simple_wait
	nop
	
@@dma_wait:

	lw		v0, D2_CHCR(a0)
	nop
	srl		v0, 24
	andi	v0, 0x1
	bnez	v0, @@dma_wait
	nop

@@gpu_wait:

	lw      v0, GP1(a0)
	nop
	srl		v0, 26
	andi	v0, 0x5
	bne		v0, 5, @@gpu_wait
	nop
	
	b		@@exit
	nop
	
@@simple_wait:					; Wait for GPU to be ready for next DMA

	lw		v0, GP1(a0)
	nop
	srl		v0, 28
	andi	v0, 0x1
	beqz	v0, @@simple_wait
	nop

@@exit:

	jr		ra
	nop
	

ex_AddPrim:

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
	
	
ex_DrawOTag:

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
	
	
label_epc:
	.asciiz	"PC="
	.align	0x4
label_op:
	.asciiz "OP="
	.align	0x4
label_badaddr:
	.asciiz	"BADDR="
	.align	0x4
label_cause:
	.asciiz	"CAUSE="
	.align	0x4
label_stack:
	.asciiz	"STACK:"
	.align	0x4
reg_labels:
	.asciiz "AT="
	.align	0x4
	.asciiz "V0="
	.align	0x4
	.asciiz "V1="
	.align	0x4
	.asciiz "A0="
	.align	0x4
	.asciiz "A1="
	.align	0x4
	.asciiz "A2="
	.align	0x4
	.asciiz "A3="
	.align	0x4
	.asciiz "T0="
	.align	0x4
	.asciiz "T1="
	.align	0x4
	.asciiz "T2="
	.align	0x4
	.asciiz "T3="
	.align	0x4
	.asciiz "T4="
	.align	0x4
	.asciiz "T5="
	.align	0x4
	.asciiz "T6="
	.align	0x4
	.asciiz "T7="
	.align	0x4
	.asciiz "S0="
	.align	0x4
	.asciiz "S1="
	.align	0x4
	.asciiz "S2="
	.align	0x4
	.asciiz "S3="
	.align	0x4
	.asciiz "S4="
	.align	0x4
	.asciiz "S5="
	.align	0x4
	.asciiz "S6="
	.align	0x4
	.asciiz "S7="
	.align	0x4
	.asciiz "T8="
	.align	0x4
	.asciiz "T9="
	.align	0x4
	.asciiz "K0="
	.align	0x4
	.asciiz "K1="
	.align	0x4
	.asciiz "GP="
	.align	0x4
	.asciiz "SP="
	.align	0x4
	.asciiz "FP="
	.align	0x4
	.asciiz "RA="
	.align	0x4
	.asciiz "HI="
	.align	0x4
	.asciiz "LO="
	.align	0x4
	
crash_msg:
	.word	crash_int
	.word	crash_tlbmod
	.word	crash_tlbload
	.word	crash_tlbstore
	.word	crash_addrload
	.word	crash_addrstore
	.word	crash_busfetch
	.word	crash_busload
	.word	crash_syscall
	.word	crash_break
	.word	crash_reserve
	.word	crash_cop
	.word	crash_overflow
	
crash_int:
	.asciiz	"INTERRUPT"
crash_tlbmod:
	.asciiz "TLB MODIFICATION"
crash_tlbload:
	.asciiz "TLB LOAD"
crash_tlbstore:
	.asciiz "TLB STORE"
crash_addrload:
	.asciiz "ADDR ERROR LOAD / INST. FETCH"
crash_addrstore:
	.asciiz "ADDR ERROR STORE"
crash_busfetch:
	.asciiz	"BUS ERROR INSTRUCTION FETCH"
crash_busload:
	.asciiz	"BUS ERROR DATA LOAD/STORE"
crash_syscall:
	.asciiz "SYSTEM CALL"
crash_break:
	.asciiz "BREAKPOINT / BREAK INSTRUCTION"
crash_reserve:
	.asciiz "RESERVED INSTRUCTION"
crash_cop:
	.asciiz "COPROCESSOR UNUSABLE"
crash_overflow:
	.asciiz "ARITHMETIC OVERFLOW"

	.align	0x4
ex_font_image:
	.incbin "exfont.tim"
	.align	0x4
