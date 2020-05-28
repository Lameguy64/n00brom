; Exception handler, for trapping unhandled exceptions

prevhook	equ	0xA000D000

ST_at		equ	0
ST_v0		equ	4
ST_v1		equ	8
ST_a0		equ	12
ST_a1		equ	16
ST_a2		equ	20
ST_a3		equ	24
ST_t0		equ	28
ST_t1		equ	32
ST_t2		equ	36
ST_t3		equ	40
ST_t4		equ	44
ST_t5		equ	48
ST_t6		equ	52
ST_t7		equ	56
ST_s0		equ	60
ST_s1		equ	64
ST_s2		equ	68
ST_s3		equ	72
ST_s4		equ	76
ST_s5		equ	80
ST_s6		equ	84
ST_s7		equ	88
ST_t8		equ	92
ST_t9		equ	96
ST_k0		equ	100
ST_k1		equ	104
ST_gp		equ	108
ST_sp		equ	112
ST_fp		equ	116
ST_ra		equ	120
ST_lo		equ	124
ST_hi		equ	128
ST_clut		equ	132
ST_temp		equ	134


install_exception:
	
	la		v1, prevhook				; Installs a CPU exception monitor
	la		a0, exception
	li		a1, (exception_end-exception)
@@copy_loop:
	lw		v0, 0(a0)
	addiu	a0, 4
	sw		v0, 0(v1) 		
	addiu	a1, -4
	bgtz	a1, @@copy_loop
	addiu	v1, 4

	li		v0, 0x80
	lw		a0, 0(v0)
	lw		a1, 4(v0)
	lw		a2, 8(v0)
	lw		a3, 12(v0)
	
	sw		a0, 0(v1)
	sw		a1, 4(v1)
	sw		a2, 8(v1)
	sw		a3, 12(v1)
	
	la		v1, 0xA0000080
	la		v0, 0x3C1A0000
	sw		v0, 0(v1)
	la		v0, 0x375AD000
	sw		v0, 4(v1)
	
	jr		ra
	nop
	
	
exception:

	nop
	nop
	nop
	nop
	li		k0, 0xC000
	
	sw		v0, ST_v0(k0)
	sw		v1, ST_v1(k0)
	
	mfc0	v0, CAUSE
	nop
	nop
	
	srl		v0, 2
	andi	v0, 0x1f
	
	li		v1, 0x8						; Let syscalls pass
	beq		v0, v1, @@return
	nop
	beqz	v0, @@return				; Trap everything else but interrupts
	nop
	
	la		k0, exception_fail
	jr		k0
	nop
	
@@return:

	lw		v0, ST_v0(k0)
	lw		v1, ST_v1(k0)
	
exception_end:

	
exception_fail:

	sw		k0, 0(r0)
	
	li		k0, 0xC000
	sw		at, ST_at(k0)
	;sw		v0, ST_v0(k0)
	;sw		v1, ST_v1(k0)
	sw		a0, ST_a0(k0)
	sw		a1, ST_a1(k0)
	sw		a2, ST_a2(k0)
	sw		a3, ST_a3(k0)
	sw		t0, ST_t0(k0)
	sw		t1, ST_t1(k0)
	sw		t2, ST_t2(k0)
	sw		t3, ST_t3(k0)
	sw		t4, ST_t4(k0)
	sw		t5, ST_t5(k0)
	sw		t6, ST_t6(k0)
	sw		t7, ST_t7(k0)
	sw		s0, ST_s0(k0)
	sw		s1, ST_s1(k0)
	sw		s2, ST_s2(k0)
	sw		s3, ST_s3(k0)
	sw		s4, ST_s4(k0)
	sw		s5, ST_s5(k0)
	sw		s6, ST_s6(k0)
	sw		s7, ST_s7(k0)
	sw		t8, ST_t8(k0)
	sw		t9, ST_t9(k0)
	lw		v0, 0(r0)
	sw		k1, ST_k1(k0)
	sw		v0, ST_k0(k0)
	sw		gp, ST_gp(k0)
	sw		sp, ST_sp(k0)
	sw		fp,	ST_fp(k0)
	sw		ra,	ST_ra(k0)
	mflo	v0
	mfhi	v1
	sw		v0, ST_lo(k0)
	sw		v1, ST_hi(k0)
	
	li		gp, 0xD000			; Set workspace area address
	
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
	addiu	a1, gp, ST_temp
	addiu	a0, gp, ST_temp
	jal		ex_drawtext
	move	a1, v0
	addiu	v0, 8
	
	la		a0, label_badaddr
	jal		ex_drawtext
	move	a1, v0
	mfc0	a0, BADVADDR
	nop
	jal		ex_reg2hex
	addiu	a1, gp, ST_temp
	addiu	a0, gp, ST_temp
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
	addiu	a1, gp, ST_temp
	addiu	a0, gp, ST_temp
	jal		ex_drawtext
	move	a1, v0
	
	la		a0, label_cause
	addiu	a1, v0, 8
	jal		ex_drawtext
	nop
	mfc0	a0, CAUSE
	nop
	jal		ex_reg2hex
	addiu	a1, gp, ST_temp
	addiu	a0, gp, ST_temp
	move	a1, v0
	jal		ex_drawtext
	nop
	
	la		s0, reg_labels
	li		s1, 0xC000
	li		s3, 0x0008
	li		s5, 3
	
@@reg_loop_col:
	
	li		s2, 0x0038
	li		s4, 11
	
@@reg_loop_row:
	
	sll		a1, s2, 16
	or		a1, s3
	
	move	a0, s0
	jal		ex_drawtext
	addiu	s0, 4
	lw		a0, 0(s1)
	addiu	s1, 4
	jal		ex_reg2hex
	addiu	a1, gp, ST_temp
	addiu	a0, gp, ST_temp
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

	move	s0, sp
	li		s1, 0x00A0
	li		s3, 8
	
@@stack_row_loop:

	li		s2, 4
	sll		v0, s1, 16
	ori		v0, 8
	
@@stack_col_loop:
	
	lw		a0, 0(s0)
	addiu	s0, 4
	jal		ex_reg2hex
	addiu	a1, gp, ST_temp
	addiu	a0, gp, ST_temp
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
	.asciiz "LO="
	.align	0x4
	.asciiz "HI="
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
