; Low-level CD routines

cd_setup:
	
	lui		a3, 0x1F80				; Acknowledge all CD IRQs
	li		v0, 1
	sb		v0, CD_REG0(a3)
	li		v0, 0x1f
	sb		v0, CD_REG3(a3)
	sb		v0, CD_REG2(a3)			; Enable all IRQs
	
	sb		r0, CD_REG0(a3)
	sb		r0, CD_REG3(a3)
	
	li		v0, 0x1325
	sw		v0, COM_DELAY(a3)
	
	jr		ra
	nop
	
	
cd_init:

	addiu	sp, -4
	sw		ra, 0(sp)
	
	li		a0, 0x01				; Getstat
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	li		a0, 0x01				; Getstat
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	li		a0, 0x0A				; Init
	jal		cd_control
	move	a2, r0
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
cd_wait:
	lbu		v0, VAR_cdinum(gp)
	nop
	blt		v0, a0, cd_wait
	nop
	jr		ra
	nop


cd_control:

	; a0 - command value
	; a1 - pointer to parameters
	; a2 - length of parameters
	
	sb		a0, VAR_cdcmd(gp)
	sb		r0, VAR_cdiwait(gp)
	sh		r0, VAR_cdinum(gp)
	
	lui		v1, 0x1F80
	
	li		v0, 1					; Clear parameter FIFO
	sb		v0, CD_REG0(v1)
	li		v0, 0x40
	sb		v0, CD_REG3(v1)
	
@@cmd_wait:							; Wait for CD to become ready for commands
	lbu		v0, CD_REG0(v1)
	nop
	andi	v0, 0x80
	bnez	v0, @@cmd_wait
	nop
	
	sb		r0, CD_REG0(v1)
	
	beqz	a2, @@no_params
	nop
	
@@feed_params:						; Feed parameters to parameter FIFO
	lbu		v0, 0(a1)
	addi	a2, -1
	sb		v0, CD_REG2(v1)
	bgtz	a2, @@feed_params
	addiu	a1, 1
	
@@no_params:

	sb		r0, CD_REG0(v1)			; Feed command value
	sb		a0, CD_REG1(v1)
	
	jr		ra
	nop
