;PrintDev:
;	addiu	t2, r0, 0xB0
;	jr		t2
;	addiu	t1, r0, 0x49
	
rom_menu:
	
	addiu	sp, -4
	sw		ra, 0(sp)
	
	;jal		PrintDev
	;nop
	
	move	s0, r0						; Selected item
	lhu		s1, VAR_padbuff(gp)
	
	la		a0, config
	addiu	a1, gp, VAR_cdresp
	lw		v0, 0(a0)
	lw		v1, 4(a0)
	sw		v0, 0(a1)
	lw		v0, 8(a0)
	sw		v1, 4(a1)
	sw		v0, 8(a1)
	
	addiu	sp, -48
	move	s2, sp
	
	jal		CartIdentify
	nop
	move	s4, v0
	
	jal		ChipIdentify
	move	a0, r0
	move	s3, v0
	
@@loop:

	jal		prep_frame
	nop
	
	la		a0, text_rom_menu
	jal		DrawText_centered
	li		a1, 0x18
	
	la		a0, list_menu_items
	li		a1, 32
	jal		DrawMenu
	li		a2, 40
	
	la		a0, list_menu_options
	li		a1, 296
	li		a2, 40
	jal		DrawMenuItems
	addiu	a3, gp, VAR_cdresp
	
	
	sb		r0, 0(s2)				; Output EEPROM string
	la		a1, text_chip
	move	a0, s2
	jal		strcpy
	addiu	sp, -8
	addiu	sp, 8
	
	move	a0, s3
	jal		ChipString
	move	a1, s2
	
	li		a0, 32
	li		a1, 180
	jal		DrawText
	move	a2, s2
	
	
	move	a0, s2					; Output detected cartridge name
	la		a1, text_cart
	jal		strcpy
	addiu	sp, -8
	addiu	sp, 8
	
	la		v1, list_carts
	sll		v0, s4, 2
	addu	v0, v1
	lw		a1, 0(v0)
	move	a0, s2
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	

	li		a0, 32
	li		a1, 188
	jal		DrawText
	move	a2, s2
	
	la		a2, build_string
	li		a0, 32
	jal		DrawText
	li		a1, 204
	
	
	addiu	t0, gp, VAR_ot			; Draw menu cursor
	lw		t1, VAR_paddr(gp)
	
	lui		v0, 0x0300				; Primitive tag
	sw		v0, 0(t1)
	lw		a1, 8(sp)
	lui		v0, 0x747f				; Color+code
	ori		v0, 0x7f7f
	sw		v0, 4(t1)
	
	li		a0, 24
	sll		a1, s0, 3				; Equivalent to multiply by 8
	addi	a1, 40
	
	la		a2, list_menu_items
	move	a3, r0
@@list_scan:
	lbu		v0, 4(a2)
	addiu	a3, 1
	beqz	v0, @@not_space
	nop
	addiu	a1, 8
@@not_space:
	blt		a3, s0, @@list_scan
	addiu	a2, 8

	sll		v0, a1, 16				; X,Y
	or		v0, a0
	sw		v0, 8(t1)
	
	lhu		v1, VAR_clut(gp)		; UV+CLUT
	li		v0, 0x5838
	sll		v1, 16
	or		v0, v1
	sw		v0, 12(t1)
	
	move	a0, t0
	jal		AddPrim
	move	a1, t1
	
	addiu	t1, 20
	sw		t1, VAR_paddr(gp)
	
	
	la		a0, 0x00100010
	la		a1, 0x00D00120
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0
	
	addiu	a1, gp, VAR_padbuff
	addiu	a2, r0, 34
	jal		PadRead
	move	a0, r0
	
	lhu		v0, VAR_padbuff(gp)
	li		at, 0x4100
	bne		v0, at, @@no_pad
	nop
	lhu		v0, VAR_padbuff+2(gp)
	nop
	beq		v0, s1, @@no_pad
	move	s1, v0
	
	andi	v1, v0, 0x10			; Up button
	beqz	v1, @@menu_up
	nop
	andi	v1, v0, 0x40			; Down button
	beqz	v1, @@menu_down
	nop
	
	andi	v1, v0, 0x80			; Left button
	beqz	v1, @@menu_left
	nop
	andi	v1, v0, 0x20			; Right button
	beqz	v1, @@menu_right
	nop
	
	andi	v1, v0, 0x4000			; Cross button
	beqz	v1, @@menu_cross
	nop
	
	b		@@no_pad
	nop
	
@@menu_up:
	addiu	s0, -1
	bgez	s0, @@no_pad
	nop
	b		@@no_pad
	addiu	s0, 12

@@menu_down:
	addiu	s0, 1
	ble		s0, 11, @@no_pad
	nop
	b		@@no_pad
	addiu	s0, -12

@@menu_left:
	addiu	v0, gp, VAR_cdresp
	addu	v0, s0
	lbu		v1, 0(v0)
	nop
	beqz	v1, @@no_pad
	addiu	v1, -1
	b		@@no_pad
	sb		v1, 0(v0)
	
@@menu_right:
	la		v0, list_menu_items
	sll		v1, s0, 3
	addu	v0, v1
	addiu	v0, 4
	lbu		a0, 2(v0)
	addiu	v0, gp, VAR_cdresp
	addu	v0, s0
	lbu		v1, 0(v0)
	addiu	a0, -1
	bge		v1, a0, @@no_pad
	addiu	v1, 1
	sb		v1, 0(v0)
	
@@menu_cross:
	
	; Handle menu choice (more flexible to compare against string addresses)
	
	la		v0, list_menu_items
	sll		v1, s0, 3				; Equivalent of multiply by 8
	addu	v0, v1
	lw		v0, 0(v0)
	
	beq		v0, list_flash, @@flash_util
	nop
	beq		v0, list_save, @@save_settings	; Save settings
	nop
	beq		v0, list_exit, @@exit			; Exit option
	move	v0, r0
	
@@no_pad:
	
	b		@@loop
	nop
	
@@save_settings:

	jal		ChipPageSize
	move	a0, s3
	
	blez	v0, @@no_pad
	nop
	
	lui		a0, 0x8001
	ori		a0, 0x0200
	move	a1, v0
@@zero_area:
	sw		r0, 0(a0)
	addiu	a1, -4
	bgtz	a1, @@zero_area
	addiu	a0, 4
	
	EnterCriticalSection
	
	jal		ChipPageSize
	move	a0, s3
	move	a3, v0
	
	lui		a0, 0x8001
	ori		a0, 0x0200
	lw		v0, VAR_cdresp+0(gp)
	lw		v1, VAR_cdresp+4(gp)
	sw		v0, 0(a0)
	lw		v0, VAR_cdresp+8(gp)
	sw		v1, 4(a0)
	sw		v0, 8(a0)
	
	la		a1, config
	jal		ChipWrite
	move	a2, a3
	
	ExitCriticalSection
	
	b		@@exit
	addiu	v0, r0, 1
	
@@flash_util:

	jal		ChipPageSize			; Don't enter flash tool if EEPROM
	move	a0, s3					; is unknown for obvious reasons
	blez	v0, @@no_pad
	nop
	addiu	v0, r0, 2
	
@@exit:

	addiu	sp, 48
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
