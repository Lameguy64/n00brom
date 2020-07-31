; system error function hooks

install_syserr:

	li		v0, 0x200				; Hooks SystemErrorBootOrDiskFailure()
	li		v1, 0xA1
	sll		v1, 2
	addu	v0, v1
	la		v1, syserr_bootdisk
	sw		v1, 0(v0)
	
	jr		ra
	nop
	
	
syserr_bootdisk:
	
	addiu	sp, -12
	move	s0, sp
	sw		r0, 0(s0)
	sw		r0, 4(s0)
	sw		r0, 8(s0)
	addiu	sp, -12
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	sw		a1, 8(sp)
	
	jal		ex_InitGPU
	nop
	
	la		a0, ex_font_image
	jal		ex_LoadTIM
	move	a1, r0
	
	lui		v1, 0x1f80
	lui		v0, 0x027f				; FillVRAM command
	sw		v0, GP0(v1)
	sw		r0, GP0(v1)
	lui		v0, 0x0100				; y = 256
	ori		v0, 0x0140				; x = 320
	sw		v0, GP0(v1)
	jal		ex_WaitGPU
	nop
	
	jal		ex_ShowDisplay
	nop
	
	la		a0, text_syserr_bootdisk
	la		a1, 0x00100008
	jal		ex_drawtext
	nop
	
	lw		a0, 4(sp)
	li		v0, ' '
	sb		v0, 1(s0)
	sb		a0, 0(s0)
	
	lw		a0, 8(sp)
	jal		ex_reg2hex
	addiu	a1, s0, 2
	
	move	a0, s0
	la		a1, 0x00200008
	jal		ex_drawtext
	nop
	
	la		a0, reg_labels+(30<<2)
	la		a1, 0x00300008
	jal		ex_drawtext
	nop
	
	lw		a0, 0(sp)
	jal		ex_reg2hex
	move	a1, s0
	la		a1, 0x00300020
	jal		ex_drawtext
	move	a0, s0
	
@@endless:
	b		@@endless
	nop


text_syserr_bootdisk:
	.asciiz "SYSTEM ERROR BOOT OR DISC FAIL"
	.align	4
