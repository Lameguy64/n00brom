unlock_cdrom:						;; HC05 secret unlock boot method

	addiu	sp, -4
	sw		ra, 0(sp)
	
	la		a0, text_unlock_cd
	jal		sort_status
	nop

	sw		r0, VAR_cdresp(gp)
	sw		r0, VAR_cdresp+4(gp)
	sw		r0, VAR_cdresp+8(gp)
	sw		r0, VAR_cdresp+12(gp)

	li		a0, 0x19
	la		a1, getid_cmd
	jal		cd_control
	li		a2, 1
	jal		cd_wait
	li		a0, 1

	addiu	a0, gp, VAR_cdresp		; Is region string US?
	la		a1, s_uscan_chk
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	
	bnez	v0, @@not_us
	nop
	b		@@is_compatible
	li		v0, 0
	
@@not_us:

	addiu	a0, gp, VAR_cdresp		; Is region string EU?
	la		a1, s_europe_chk
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8

	bnez	v0, @@not_eu
	nop
	b		@@is_compatible
	li		v0, 1
	
@@not_eu:
	
	addiu	a0, gp, VAR_cdresp		; Is region string Worldwide (Yaroze)?
	la		a1, s_netna_chk
	jal		strcmp
	addiu	sp, -8
	addiu	sp, 8
	
	bnez	v0, @@unlock_not_support
	nop
	
	li		v0, 2

@@is_compatible:
	
	addiu	sp, -4
	sw		v0, 0($sp)
	
	li		a0, 0x50				; Unlock pt1
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	la		a0, s_unlock_a			; Unlock pt2
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x51
	la		a1, s_unlock_a
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1
	
	la		a0, s_unlock_b			; Unlock pt3
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x52
	la		a1, s_unlock_b
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1
	
	la		a0, s_unlock_c			; Unlock pt4
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x53
	la		a1, s_unlock_c
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1

	la		a0, s_unlock_d			; Unlock pt5
	jal		strlen
	addiu	sp, -4
	addiu	sp, 4
	li		a0, 0x54
	la		a1, s_unlock_d
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1
	
	lw		v0, 0(sp)
	addiu	sp, 4
	
	beq		v0, 0, @@unlock_us
	nop
	beq		v0, 1, @@unlock_eu
	nop
	
	la		a0, s_unlock_e_ww			; Unlock Worldwide (yaroze)
	jal		strlen
	addiu	sp, -4
	la		a1, s_unlock_e_ww
	b		@@unlock_end
	addiu	sp, 4
	
@@unlock_us:

	la		a0, s_unlock_e_us			; Unlock US
	jal		strlen
	addiu	sp, -4
	la		a1, s_unlock_e_us
	b		@@unlock_end
	addiu	sp, 4

@@unlock_eu:

	la		a0, s_unlock_e_eu			; Unlock EU
	jal		strlen
	addiu	sp, -4
	la		a1, s_unlock_e_eu
	b		@@unlock_end
	addiu	sp, 4
	
@@unlock_end:

	li		a0, 0x55
	jal		cd_control
	move	a2, v0
	jal		cd_wait
	li		a0, 1

	li		a0, 0x56					; Last unlock command
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1

	lw		ra, 0(sp)					; Return with non-zero value
	addiu	sp, 4
	jr		ra							
	addiu	v0, r0, 1
	
@@unlock_not_support:					; If unlock not supported (Japan)

	jal		prep_frame
	nop
	
	la		a0, text_unlock_incompat
	jal		DrawText_multiline
	li		a1, 0x5c
	
	la		a0, text_return
	jal		DrawText_centered
	li		a1, 0x8c
	
	la		a0, 0x00500020
	la		a1, 0x00500100
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
	andi	v0, 0x4000				; Cross button
	beqz	v0, @@exit
	nop
	
@@no_pad:
	
	b		@@unlock_not_support
	nop
	
@@exit:
	
	lw		ra, 0(sp)				; Return with non-zero value
	addiu	sp, 4
	jr		ra							
	move	v0, r0
	

swap_trick:							;; Ez-swap boot method

	addiu	sp, -8
	sw		ra, 0(sp)
	
@@repeat_getid:

	li		a0, 0x1A				; GetID
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	lbu		v0, VAR_cdival(gp)		; Repeat command if first response is INT5
	nop
	andi	v0, 0x7
	beq		v0, 0x5, @@repeat_getid
	nop
	
	jal		cd_wait					; Wait for second response
	li		a0, 2
	
	lbu		v0, VAR_cdival(gp)
	nop
	andi	v0, 0x7
	beq		v0, 0x5, swap_no_disc
	nop
	
	lbu		v0, VAR_cdresp+2(gp)	; Check if a Audio CD was stuffed
	nop								; on a chipped console
	beqz	v0, swap_no_disc
	nop
	
	li		a0, 0x08				; Stop
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 2
	
disc_stopped:
	
	addiu	a1, gp, VAR_padbuff
	addiu	a2, r0, 34
	jal		PadRead
	move	a0, r0
	
	lhu		v0, VAR_padbuff(gp)
	li		at, 0x4100
	bne		v0, at, @@loop
	nop
	lhu		v0, VAR_padbuff+2(gp)
	nop
	andi	v0, 8					; Start button
	beqz	v0, disc_stopped
	nop
	
@@loop:

	jal		prep_frame
	nop
	
	la		a0, text_swap
	jal		DrawText_multiline
	li		a1, 0x60
	
	la		a0, text_continue
	jal		DrawText_centered
	li		a1, 0x80
	
	la		a0, 0x00500020
	la		a1, 0x00500100
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
	andi	v0, 8					; Start button
	beqz	v0, swap_start_disc
	nop
	
@@no_pad:
	
	b		@@loop
	nop
	
swap_start_disc:
	
	li		a0, 0x07				; MotorOn
	jal		cd_control
	move	a2, r0
	
@@loop:

	la		a0, text_check_cd
	jal		sort_status
	nop

	lbu		v0, VAR_cdinum(gp)		; Wait until two CD IRQs occur
	nop
	blt		v0, 2, @@loop
	nop
	
	li		a0, 0x12				; SetSession (acts like ReadTOC but doesn't
	addiu	a1, r0, 1				; clear authentication status, thanks rama)
	sw		a1, 4(sp)
	addiu	a1, sp, 4
	jal		cd_control
	addiu	a2, r0, 1
	
@@loop_2:
	la		a0, text_check_cd
	jal		sort_status
	nop
	
	lbu		v0, VAR_cdinum(gp)		; Wait until two CD IRQs occur
	nop
	blt		v0, 2, @@loop_2
	nop
	
@@repeat_getid:

	li		a0, 0x1A				; GetID
	jal		cd_control
	move	a2, r0
	jal		cd_wait
	li		a0, 1
	
	lbu		v0, VAR_cdival(gp)		; Repeat command if first response is INT5
	nop
	andi	v0, 0x7
	beq		v0, 0x5, @@repeat_getid
	nop
	
	jal		cd_wait					; Wait for second response
	li		a0, 2
	
	lbu		v0, VAR_cdival(gp)
	nop
	andi	v0, 0x7
	beq		v0, 0x5, swap_no_disc
	nop

	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	addi	v0, r0, 1
	
swap_no_disc:
	
	addiu	a1, gp, VAR_padbuff
	addiu	a2, r0, 34
	jal		PadRead
	move	a0, r0
	
	lhu		v0, VAR_padbuff(gp)
	li		at, 0x4100
	bne		v0, at, @@loop
	nop
	lhu		v0, VAR_padbuff+2(gp)
	nop
	andi	v0, 8					; Start button
	beqz	v0, swap_no_disc
	nop
	
@@loop:

	jal		prep_frame
	nop
	
	la		a0, text_no_disc
	jal		DrawText_centered
	li		a1, 0x6c
	
	la		a0, text_retry
	jal		DrawText_centered
	li		a1, 0x7c
	
	la		a0, 0x00500020
	la		a1, 0x00500100
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
	andi	v0, 8					; Start button
	beqz	v0, init_disc
	nop
@@no_pad:
	
	b		@@loop
	nop


getid_cmd:
.byte	0x22
.align 4
