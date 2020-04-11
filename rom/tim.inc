TIM_ID		equ	0
TIM_FLAG	equ	4
TIM_DATA	equ	8

TIMBLK_SIZE	equ	0
TIMBLK_DX	equ	4
TIMBLK_DY	equ	6
TIMBLK_W	equ	8
TIMPLK_H	equ	10
TIMBLK_SLEN	equ	12

LoadTIM:

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
	
	sw		v0, 0(a1)
	move	a1, v0
	
	jal		LoadImage_IO		; Upload to SGRAM/VRAM
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
	
	jal		LoadImage_IO		; Upload to SGRAM/VRAM
	subi	a3, TIMBLK_SLEN

	lw		ra, 0(sp)
	lw		a0, 4(sp)
	jr		ra
	addiu	sp, 8
