; General graphics routines

; a0 - 0-draw background, 1-don't draw background
frame_done:
	addiu	sp, -4
	sw		ra, 0(sp)

	bnez	a0, @@no_background
	nop
	
	la		v0, config
	lbu		v0, SET_bg(v0)
	nop
	
	bnez	v0, @@skip_plasma
	nop
	
	lw		a1, VAR_vsync(gp)
	addiu	sp, -357
	jal		genPlasma
	move	a0, sp
	
	addiu	a0, gp, VAR_ot
	lw		a1, VAR_paddr(gp)
	jal		sortPlasma
	move	a2, sp
	sw		v0, VAR_paddr(gp)
	
	b		@@background_done
	addiu	sp, 357
	
@@skip_plasma:

	bne		v0, 1, @@skip_bars
	nop
	
	addiu	a0, gp, VAR_ot
	lw		a1, VAR_paddr(gp)
	jal		sortColorBars
	nop
	
@@skip_bars:
	b		@@background_done
	nop
	
@@no_background:

	move	a0, r0
	la		a1, 0x01000140
	jal		sort_rect_solid
	move	a2, r0	

@@background_done:
	
	jal		DrawOTag
	addiu	a0, gp, VAR_ot
	jal		DrawSync
	nop
	jal		VSync
	nop	
	lbu		a0, VAR_db(gp)
	jal		SetDrawPage
	nop
	
	lbu		a0, VAR_db(gp)			; Alternate buffers
	nop
	xori	a0, a0, 1
	sb		a0, VAR_db(gp)
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
prep_frame:

	lui		v0, 0x00FF
	ori		v0, 0xFFFF
	sw		v0, VAR_ot(gp)
	addiu	v0, gp, VAR_pbuff
	jr		ra
	sw		v0, VAR_paddr(gp)
	

; a0 - Status string	
sort_status:
	
	addiu	sp, -8
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	
	jal		prep_frame
	nop
	
	lw		a0, 4(sp)
	jal		DrawText_centered
	li		a1, 0x74
	
	la		a0, 0x00500040
	la		a1, 0x005000C0
	jal		sort_rect
	move	a2, r0
	
	jal		frame_done
	move	a0, r0

	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	nop

	
; a0 - XY coords
; a1 - WH coords
; a2 - Color	
sort_rect:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	lw		t0, VAR_paddr(gp)
	lui		v0, 0x0300
	sw		v0, 0(t0)
	
	lui		v0, 0x6200
	or		v0, a2
	sw		v0, 4(t0)
	sw		a0, 8(t0)
	sw		a1, 12(t0)
	
	addiu	a0, gp, VAR_ot
	jal		AddPrim
	move	a1, t0
	
	addiu	t0, 16
	sw		t0, VAR_paddr(gp)
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
sort_rect_solid:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	lw		t0, VAR_paddr(gp)
	lui		v0, 0x0300
	sw		v0, 0(t0)
	
	lui		v0, 0x6000
	or		v0, a2
	sw		v0, 4(t0)
	sw		a0, 8(t0)
	sw		a1, 12(t0)
	
	addiu	a0, gp, VAR_ot
	jal		AddPrim
	move	a1, t0
	
	addiu	t0, 16
	sw		t0, VAR_paddr(gp)
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
; a0 - input text
; a1 - X position
; a2 - Y position
DrawMenu:
	addiu	sp, -16
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	sw		s2, 12(sp)
	
	move	s0, a0
	move	s1, a1
	move	s2, a2

@@loop:
	
	lw		a2, 0(s0)		; Load pointer to string
	move	a0, s1
	
	beqz	a2, @@end
	nop
	
	jal		DrawText
	move	a1, s2
	
	lw		a0, 0(s0)
	nop
@@search_line:
	lb		v0, 0(a0)
	addiu	a0, 1
	beqz	v0, @@next_item
	nop
	bne		v0, 0x10, @@search_line
	nop
	
	addiu	s2, 8
	
@@next_item:

	addiu	s2, 8
	b		@@loop
	addiu	s0, 8
	
@@end:

	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	lw		s2, 12(sp)
	jr		ra
	addiu	sp, 16
	

; a0 - input text
; a1 - X position
; a2 - Y position
; a3 - Options
DrawMenuItems:
	addiu	sp, -20
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	sw		s2, 12(sp)
	sw		s3, 16(sp)
	
	move	s0, a0
	move	s1, a1
	move	s2, a2
	move	s3, a3
	
@@loop:
	
	lw		a2, 0(s0)		; Load pointer to options
	nop
	
	beqz	a2, @@end
	nop
	
	lbu		v0, 0(s3)		; Get option value
	nop
	sll		v0, 2
	addu	a2, v0
	lw		a0, 0(a2)
	
	jal		strlen
	nop
	lw		a2, 0(s0)
	sll		v0, 3
	sub		a0, s1, v0
	lbu		v0, 0(s3)
	nop
	sll		v0, 2
	addu	a2, v0
	lw		a2, 0(a2)
	
	jal		DrawText
	move	a1, s2

	addiu	s3, 1
	addiu	s2, 8
	b		@@loop
	addiu	s0, 4
	
@@end:

	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	lw		s2, 12(sp)
	lw		s3, 16(sp)
	jr		ra
	addiu	sp, 20
	
	
; a0 - input text
; a1 - Y position
DrawText_multiline:
	addiu	sp, -12
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	
	move	s0, a0
	move	s1, a1
	
@@loop:

	move	a0, s0
	jal		DrawText_centered
	move	a1, s1
	
@@search_line:
	lb		v0, 0(s0)
	addiu	s0, 1
	beqz	v0, @@end
	nop
	bne		v0, 0x10, @@search_line
	nop
	
	b		@@loop
	addiu	s1, 8
	
@@end:
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	jr		ra
	addiu	sp, 12
	
; a0 - text
; a1 - Y position
DrawText_centered:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	move	v1, a0
	move	v0, r0
@@str_count:
	lbu		t0, 0(v1)
	addiu	v0, 1
	beq		t0, 0x10, @@end_count
	nop
	bnez	t0, @@str_count
	addiu	v1, 1
@@end_count:
	
	move	a2, a0
	
	sll		v1, v0, 2
	li		a0, 160
	sub		a0, v1
	
	jal		DrawText
	subi	a0, -4
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	
	
; a0 - X
; a1 - Y
; a2 - Text
DrawText:
	
	addiu	sp, -12
	sw		ra, 0(sp)
	sw		a0, 4(sp)
	sw		a1, 8(sp)
	
	addiu	t0, gp, VAR_ot
	lw		t1, VAR_paddr(gp)
	
@@char_loop:
	
	lbu		a3, 0(a2)
	nop
	beqz	a3, @@end				; End if null terminator encountered
	nop
	beq		a3, 0x10, @@end			; End if line feed character
	nop
	
	addiu	a3, -32
	
	blez	a3, @@skip				; Skip if invalid character or space
	nop
	
	lui		v0, 0x0300
	sw		v0, 0(t1)
	
	lw		a1, 8(sp)
	
	lui		v0, 0x747f
	ori		v0, 0x7f7f
	sw		v0, 4(t1)
	
	li		at, 8
	divu	a3, at
	
	sll		v0, a1, 16
	or		v0, a0
	sw		v0, 8(t1)
	
	mflo	v1
	mfhi	v0
	
	sll		v0, 3
	sll		v1, 11
	or		v0, v1
	
	lhu		v1, VAR_clut(gp)
	nop
	sll		v1, 16
	or		v0, v1
	sw		v0, 12(t1)
	
	move	a0, t0
	jal		AddPrim
	move	a1, t1
	
	lw		a0, 4(sp)
	addiu	t1, 20
	
@@skip:

	addiu	a0, 8
	sw		a0, 4(sp)
	
	b		@@char_loop
	addiu	a2, 1
	
@@end:

	sw		t1, VAR_paddr(gp)

	lw		ra, 0(sp)
	addiu	sp, 12
	jr		ra
	nop
	
	
VSync:

	lw		v0, VAR_vsync(gp)
	nop

@wait_sync:

	lw		v1, VAR_vsync(gp)
	nop
	beq		v0, v1, @wait_sync
	nop
	
	jr		ra
	nop
