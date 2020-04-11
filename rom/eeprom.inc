; PAR EEPROM mappings:
; 1F000000h-1F01FFFFh - First 128K
; 1F040000h-1F05FFFFh - Next 128K or second 128K EEPROM

; Xplorer EEPROM mappings:
; 1F000000h-1F03FFFFh - First 256K
; 1F040000h-1F05FFFFh - Additional 256K (FX), can be remapped to SRAM


; Returns:
;	0 - PAR/GS (single EEPROM)
;	1 - PAR/GS (dual EEPROM)
;	2 - Xplorer
;	3 - Xplorer FX
;	4 - Xplorer FX (dual EEPROM)
;	5 - Xplorer FX (modded with 256K EEPROM)

CART_par				equ	0
CART_par_dual			equ	1
CART_xplorer			equ	2
CART_xplorerfx			equ	3
CART_xplorerfx_dual		equ	4
CART_xplorerfx_modded	equ	5
CART_unknown			equ	6

CartIdentify:

	addiu	sp, -8
	sw		ra, 0(sp)
	
	jal		chipid_enter
	move	a0, r0

	lui		a0, 0x1F00
	lhu		a0, 0(a0)

	lui		a0, 0x1F03					; If 0x1F030000 segment is open bus,
	lw		v0, 0(a0)					; It's a PAR
	lw		v1, 16(a0)
	addiu	a0, r0, -1
	or		v0, v1
	beq		v0, a0, @@is_par
	nop

	;; Cart detection for single chip Xplorers

	lui		a0, 0x1F00
	lhu		v0, 0(a0)
	lui		a0, 0x1F04
	lhu		v1, 0(a0)
	nop
	beq		v0, v1, @@is_xp			; If equal, single EEPROM Xplorer FX
	nop
	
	;; Cart detection for single chip PAR/GS
	
	jal		chipid_exit
	move	a0, r0
	
	b		@@exit
	li		v0, CART_unknown
	
@@is_par:
	
	lui		a0, 0x1F00
	lhu		v0, 0(a0)
	jal		chipid_exit
	move	a0, r0
	
	sw		v0, 4(sp)

	lui		a0, 0x1F00					; Check if second EEPROM address mirrors
	lw		v0, 4(a0)
	lui		a0, 0x1F04
	lw		v1, 4(a0)
	nop
	beq		v0, v1, @@exit				; If mirrored under 128KB, single EEPROM
	li		v0, CART_par
	
	lw		a0, 4(sp)
	jal		ChipSize
	nop
	beq		v0, 262144, @@exit			; If EEPROM is 128K, single chip PAR/GS
	li		v0, CART_par
	
	b		@@exit
	li		v0, CART_par_dual
	

@@is_xp:

	lui		a0, 0x1F00
	lhu		v0, 0(a0)
	
	jal		chipid_exit					; Exit ChipID mode
	move	a0, r0
	
	jal		ChipSize
	move	a0, v0
	
	bgt		v0, 262144, @@exit			; If EEPROM is >256K, it is an FX
	li		v0, CART_xplorerfx
	
	lui		a0, 0x1F00					; Check if >256K is a mirror
	lw		v0, 4(a0)
	lui		a0, 0x1F04
	lw		v1, 4(a0)
	nop
	bne		v0, v1, @@exit				; If not mirror, it is dual EEPROM
	li		v0, CART_xplorerfx_dual
	
	lui		a0, 0x1F06					; Enable SRAM
	li		v1, 0x50
	sb		v1, 1(a0)
	
	lui		a0, 0x1F04					; Test presence of SRAM if modded FX
	li		a1, 0xDEADBEEF
	sw		a1, 0(a0)
	nop
	nop
	lw		a2, 0(a0)
	
	lui		a0, 0x1F06
	beq		a1, a2, @@xpfx_modded
	sb		r0, 1(a0)
	
	b		@@exit
	li		v0, CART_xplorer
	
@@xpfx_modded:
	
	li		v0, CART_xplorerfx_modded
	
@@exit:
	
	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	nop
	

chipid_enter:

	; a0 - offset
	
	lui		v1, 0x1F00					; Enter chip identification mode
	addu	v1, a0
	li		a1, 0xAA
	sb		a1, 0x5555(v1)
	li		a1, 0x55
	sb		a1, 0x2AAA(v1)
	li		a1, 0x90
	sb		a1, 0x5555(v1)
	
	lui		v1, 0xBFC0					; Delay
	li		a0, 0x1000
@@delay_loop:
	lw		t0, 4(v1)
	lw		t1, 0(v1)
	bgtz	a0, @@delay_loop
	addi	a0, -1
	
	jr		ra
	nop
	
	
chipid_exit:

	; a0 - offset
	
	lui		v1, 0x1F00
	li		a1, 0xAA					; Exit identification mode
	sb		a1, 0x5555(v1)
	li		a1, 0x55
	sb		a1, 0x2AAA(v1)
	li		a1, 0xF0
	sb		a1, 0x5555(v1)
	
	; Delay
	lui		v1, 0xBFC0
	li		a0, 0x1000
@@delay_loop:
	lw		t0, 4(v1)
	lw		t1, 0(v1)
	bgtz	a0, @@delay_loop
	addi	a0, -1
	
	jr		ra
	nop
	

ChipWrite:
	
	lui		v1, 0x1F00
	li		v0, 0xAA
	sb		v0, 0x5555(v1)
	li		v0, 0x55
	sb		v0, 0x2AAA(v1)
	li		v0, 0xA0
	sb		v0, 0x5555(v1)
	
	lui		v0, 0x1F00
	or		a1, v0
	
	lui		v1, 0xBFC0			; Write sector
@@write_loop:
	lbu		v0, 0(a0)
	addi	a2, -1
	sb		v0, 0(a1)
	addiu	a0, 1
	lw		v0, 4(v1)
	lw		v0, 0(v1)
	bgtz	a2, @@write_loop
	addiu	a1, 1
	
	li		a2, 0x1000			; Delay
@@write_delay_loop:
	lw		v0, 4(v1)
	lw		v0, 0(v1)
	bgtz	a2, @@write_delay_loop
	addi	a2, -1
	
	lbu		a0, -1(a1)			; Poll for write confirm
	nop
	andi	a0, 0x40
@@poll_loop:
	lw		v0, 4(v1)
	lw		v0, 0(v1)
	lbu		a2, -1(a1)
	nop
	andi	a2, 0x40
	bne		a0, a2, @@poll_loop
	nop
	
	jr		ra
	nop
	
	
ChipIdentify:

	; Chip identification sequence
	; a0 - base address offset
	
	addiu	sp, -4
	sw		a0, 0(sp)
	
	EnterCriticalSection

	lw		a0, 0(sp)
	addiu	sp, 4
		
	lui		v1, 0x1F00
	li		a1, 0xAA
	sb		a1, 0x5555(v1)
	li		a1, 0x55
	sb		a1, 0x2AAA(v1)
	li		a1, 0x90
	sb		a1, 0x5555(v1)

	; Delay
	lui		v1, 0xBFC0
	li		a2, 0x1000
@@delay_loop:
	lw		t0, 4(v1)
	lw		t1, 0(v1)
	bgtz	a2, @@delay_loop
	addi	a2, -1
	
	; Get ID
	lui		v1, 0x1F00
	addu	v1, a0
	lhu		v0, 0(v1)
	
	li		a1, 0xAA			; Chip identification exit sequence
	sb		a1, 0x5555(v1)
	li		a1, 0x55
	sb		a1, 0x2AAA(v1)
	li		a1, 0xF0
	sb		a1, 0x5555(v1)
	
	; Delay
	lui		v1, 0xBFC0
	li		a2, 0x1000
@@delay_loop_2:
	lw		t0, 4(v1)
	lw		t1, 0(v1)
	bgtz	a2, @@delay_loop_2
	addi	a2, -1
	
	li		a1, 0xAA			; Chip identification exit sequence
	sb		a1, 0x5555(v1)
	li		a1, 0x55
	sb		a1, 0x2AAA(v1)
	li		a1, 0xF0
	sb		a1, 0x5555(v1)
	
	; Delay
	lui		v1, 0xBFC0
	li		a2, 0x4000
@@delay_loop_3:
	lw		t0, 4(v1)
	lw		t1, 0(v1)
	bgtz	a2, @@delay_loop_3
	addi	a2, -1
	
	ExitCriticalSection
	
	jr		ra
	nop


ChipSize:
	
	; a0 - Chip vendor+model code
	; return: EEPROM capacity in bytes, -1 if unknown
	
	la		t0, chipid_list
	
@@search_loop:
	lhu		v0, 0(t0)
	nop
	beqz	v0, @@search_end
	nop
	bne		a0, v0, @@search_loop
	addiu	t0, 16
	addiu	t0, -16
	
	lw		v0, 4(t0)
		
	jr		ra
	nop
	
@@search_end:

	jr		ra
	addiu	v0, r0, -1
	
	
ChipPageSize:
	
	; a0 - Chip vendor+model code
	; return: EEPROM page size in bytes, -1 if unknown
	
	la		t0, chipid_list
	
@@search_loop:
	lhu		v0, 0(t0)
	nop
	beqz	v0, @@search_end
	nop
	bne		a0, v0, @@search_loop
	addiu	t0, 16
	addiu	t0, -16
	
	lh		v0, 2(t0)
		
	jr		ra
	nop
	
@@search_end:

	jr		ra
	addiu	v0, r0, -1
	
	
ChipString:
	; a0 - Chip vendor+model code
	; a1 - Output string (zero terminated)
	
	addiu	sp, -8
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	
	la		s0, chipid_list
@@search_loop:
	lhu		v0, 0(s0)
	nop
	beqz	v0, @@search_end
	nop
	bne		a0, v0, @@search_loop
	addiu	s0, 16
	addiu	s0, -16
	
	move	a0, a1			; Copy vendor string
	lw		a1, 8(s0)
	addiu	sp, -4
	sw		a0, 0(sp)
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, 0(sp)
	addiu	sp, 4
	
	lw		a1, 12(s0)		; Copy model string
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
	b		@@exit
	nop
	
@@search_end:
	
	; Search by vendor
	
	andi	v1, a0, 0xFF
	
	la		s0, chipid_list
@@search_loop_vendor:
	lbu		v0, 0(s0)
	nop
	beqz	v0, @@search_end_vendor
	nop
	bne		v1, v0, @@search_loop_vendor
	addiu	s0, 12
	
	addiu	sp, -8
	sw		a0, 0(sp)
	sw		a1, 4(sp)
	
	addiu	s0, -12
	move	a0, a1
	lw		a1, 4(s0)
	addiu	sp, -4
	sw		a0, 0(sp)
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	lw		a0, 0(sp)
	addiu	sp, 4
	
	la		a1, chipid_unknown
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
	lw		a0, 0(sp)
	
	addiu	sp, -12
	jal		reg2hex
	move	a1, sp
	addiu	sp, 12
	lw		a0, 4(sp)
	addiu	sp, -12
	addiu	a1, sp, 4
	li		a2, 4
	jal		strncat
	addiu	sp, -12
	addiu	sp, 12
	
	addiu	sp, 12+8
	
	b		@@exit
	nop
	
@@search_end_vendor:

	addiu	sp, -8
	sw		a0, 0(sp)
	sw		a1, 4(sp)
	
	move	a0, a1
	la		a1, chipid_unknown
	jal		strcat
	addiu	sp, -8
	addiu	sp, 8
	
	lw		a0, 0(sp)
	
	addiu	sp, -12
	jal		reg2hex
	move	a1, sp
	addiu	sp, 12
	lw		a0, 4(sp)
	addiu	sp, -12
	addiu	a1, sp, 4
	li		a2, 4
	jal		strncat
	addiu	sp, -12
	addiu	sp, 12
	
	addiu	sp, 12+8
@@exit:
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	addiu	sp, 8
	jr		ra
	nop

.align 4
