TILE_tag		equ	0
TILE_rgb0		equ	4
TILE_xy0		equ	8
TILE_wh			equ	12
TILE_len		equ 16

sortColorBars:

	move	t0, r0
	move	t1, r0
	
@@bars_loop:

	la		v0, colorbar_colors
	sll		v1, t1, 2
	addu	v0, v1
	lw		v1, 0(v0)
	sw		t0, TILE_xy0(a1)
	li		v0, 0x01000028
	sw		v0, TILE_wh(a1)
	sw		v1, TILE_rgb0(a1)
	
	lui		v1, 0x0300
	lw		v0, 0(a0)
	lui		at, 0xff00
	and		v1, at
	lui		at, 0x00ff
	ori		at, 0xffff
	and		v0, at
	or		v1, v0
	sw		v1, 0(a1)
	lw		v0, 0(a0)
	and		a1, at
	lui		at, 0xff00
	and		v0, at
	or		v0, a1
	sw		v0, 0(a0)

	lui		v0, 0x8000
	or		a1, v0
	addiu	a1, TILE_len
	
	addi	t0, 40
	blt		t1, 7, @@bars_loop
	addi	t1, 1
	
	jr		ra
	nop
	
