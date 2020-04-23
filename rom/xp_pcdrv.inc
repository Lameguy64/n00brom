DIRENT_name	equ	0
DIRENT_attr	equ	20
DIRENT_size	equ	24
DIRENT_next	equ	28
DIRENT_head	equ	32
DIRENT_sys	equ	36

IOB_flgs	equ	0
IOB_unit	equ	4
IOB_ma		equ	8
IOB_cc		equ	12
IOB_offset	equ	16
IOB_fstype	equ	20
IOB_errno	equ	24
IOB_dp		equ	28
IOB_size	equ	32
IOB_head	equ	36
IOB_fd		equ	40

PCDRV_ECSUM		equ 254

install_xp_pcdrv:
	addiu	sp, -4
	sw		ra, 0(sp)
	
	la		a0, xp_pcdrv_iob		; Add n00bROM's PCDRV device
	jal		rom_AddDev
	addiu	sp, -4
	addiu	sp, 4
	
	lw		ra, 0(sp)
	addiu	sp, 4
	jr		ra
	nop
	

xp_pcdrv_open:						;; Open call
	addiu	sp, -20
	sw		ra, 0(sp)
	sw		a0, 4(sp)				; IOB ptr
	sw		a1, 8(sp)				; Filename ptr
	sw		a2, 12(sp)				; Mode
	
	jal		xp_sendbyte_rom			; File open command
	li		a0, 0x20
	
	lw		a3, 12(sp)				; Send file mode
	jal		xp_sendbyte_rom
	andi	a0, a3, 0xFF
	srl		a0, a3, 8
	jal		xp_sendbyte_rom
	andi	a0, 0xFF
	
	lw		t0, 8(sp)				; Send file name string
	nop
@@send_string:
	lbu		a0, 0(t0)
	addiu	t0, 1
	beqz	a0, @@send_string_done
	nop
	jal		xp_sendbyte_rom
	nop
	b		@@send_string
	nop
@@send_string_done:
	jal		xp_sendbyte_rom			; Terminate string transmission
	move	a0, r0
	
	jal		xp_readbyte_rom			; Get error code
	nop
	
	lw		a0, 4(sp)				; Store error to IOB
	nop
	sw		v0, IOB_errno(a0)
	
	bnez	v0, @@exit				; Skip finishing the FCB if there's an error
	nop
	
	; Set up the IOB
	
	jal		xp_readbyte_rom			; Get file handle
	nop
	
	lw		a0, 4(sp)
	nop
	sw		v0, IOB_unit(a0)		; Store PCDRV handle as pseudo device unit
	
	jal		xp_readreg_rom			; Get file size
	nop
	lw		a0, 4(sp)
	nop
	sw		v0, IOB_size(a0)
	
	sw		r0, IOB_offset(a0)
	sw		r0, IOB_head(a0)
	
	move	v0, r0
		
@@exit:

	lw		ra, 0(sp)
	addiu	sp, 20
	jr		ra
	nop
	
	
xp_pcdrv_close:
	
	addiu	sp, -8
	sw		ra, 0(sp)
	sw		a0, 4(sp)

	jal		xp_sendbyte_rom			; File close command
	li		a0, 0x21
	
	lw		a0, 4(sp)
	nop
	jal		xp_sendbyte_rom			; Send file handle
	lbu		a0, IOB_unit(a0)
	
	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	nop


xp_pcdrv_read:
	addiu	sp, -32
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	sw		s2, 12(sp)
	sw		a0, 16(sp)
	sw		a1, 20(sp)
	sw		a2, 24(sp)

	move	s0, a0
	move	s1, a1
	move	s2, a2
	
	jal		xp_sendbyte_rom			; Send read command
	li		a0, 0x22
	
	jal		xp_sendbyte_rom			; Send file handle
	lbu		a0, IOB_unit(s0)
	
	jal		xp_sendreg_rom			; Send number of bytes to read
	move	a0, s2
	
	jal		xp_readbyte_rom			; Get error code
	nop
	sw		v0, IOB_errno(s0)
	
	jal		xp_readreg_rom			; Get read result
	nop
	
	beqz	v0, @@exit
	sw		v0, 28(sp)

	move	s0, v0					; Bytes to read
	
@@retry:							; Read receive routine
	move	a3, r0					; Checksum initial
@@read_loop:
	jal		xp_readbyte_rom
	addiu	s0, -1
	sb		v0, 0(s1)
	
	li		a1, 8					; Compute checksum (crc8)
	move	v1, v0
@@csum_loop:
	xor		v0, a3, v1				; sum = (crc ^ extract) & 0x1
	andi	v0, 1
	beqz	v0, @@not_sum
	srl		a3, 1					; crc >>= 1
	xori	a3, 0x8C
@@not_sum:
	addiu	a1, -1
	bgtz	a1, @@csum_loop
	srl		v1, 1					; extract >>= 1
	
	bgtz	s0, @@read_loop
	addiu	s1, 1
	
	jal		xp_readbyte_rom			; Get checksum
	nop
	
	beq		v0, a3, @@data_ok		; Test checksum results
	nop
	
	jal		xp_sendbyte_rom			; Retry receive on checksum error
	addiu	a0, r0, 254
	
	lw		s1, 8(sp)
	b		@@retry
	lw		s0, 28(sp)
	
@@data_ok:

	jal		xp_sendbyte_rom			; Checksum OK
	move	a0, r0
	
	lw		s0, 16(sp)
	jal		xp_readreg_rom			; Get new file offset
	nop
	
	sw		v0, IOB_offset(s0)		; Set it to IOB
	
	lw		v0, 28(sp)				; Load bytes read count
	
@@exit:
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	lw		s2, 12(sp)
	jr		ra
	addiu	sp, 32
	
	
xp_pcdrv_write:
	addiu	sp, -28
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	sw		s2, 12(sp)
	sw		a1, 16(sp)
	sw		a2, 20(sp)
	
	move	s0, a0					; FCB
	move	s1, a1					; Buffer addr
	move	s2, a2					; Len
	
	jal		xp_sendbyte_rom			; File write command
	li		a0, 0x23
	
	jal		xp_sendbyte_rom			; Send file handle
	lbu		a0, IOB_unit(s0)
	
	jal		xp_sendreg_rom			; Send file position
	lw		a0, IOB_offset(s0)
	
	jal		xp_sendreg_rom			; Send write length
	move	a0, s2
	
@@retry:

	move	a3, r0					; Send the data to be written
@@send_loop:						; and calculate checksum
	lbu		a0, 0(s1)
	
	li		a1, 8					; Compute checksum (crc8)
	move	v1, a0
@@csum_loop:
	xor		v0, a3, v1				; sum = (crc ^ extract) & 0x1
	andi	v0, 1
	beqz	v0, @@not_sum
	srl		a3, 1					; crc >>= 1
	xori	a3, 0x8C
@@not_sum:
	addiu	a1, -1
	bgtz	a1, @@csum_loop
	srl		v1, 1					; extract >>= 1
	
	jal		xp_sendbyte_rom
	addiu	s2, -1
	bgtz	s2, @@send_loop
	addiu	s1, 1
	
	
	jal		xp_sendbyte_rom			; Send checksum
	move	a0, a3
	
	jal		xp_readbyte_rom			; Get error code
	nop
	
	bne		v0, PCDRV_ECSUM, @@no_csum_err
	sw		v0, IOB_errno(s0)		; Store error code to FCB
	lw		s1, 16(sp)				; Retry transfer if checksum error
	b		@@retry
	lw		s2, 20(sp)
@@no_csum_err:

	jal		xp_readreg_rom			; Get number of bytes written
	nop
	
	jal		xp_readreg_rom			; Get new file pos
	sw		v0, 24(sp)				; Save bytes written
	
	sw		v0, IOB_offset(s0)		; Store new file position to FCB
	
	lw		v1, IOB_size(s0)		; Set pos as new file size if higher
	nop
	ble		v0, v1, @@pos_not_bigger
	nop
	sw		v0, IOB_size(s0)
@@pos_not_bigger:
	
	lw		v0, 24(sp)				; Restore bytes written as return value
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	lw		s2, 12(sp)
	jr		ra
	addiu	sp, 28


xp_pcdrv_ffile:
	addiu	sp, -16
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	sw		a2, 12(sp)
	
	move	s0, a0
	move	s1, a1
	
	jal		xp_sendbyte_rom			; Send first file command
	addiu	a0, r0, 0x25
	
@@send_loop:						; Send path and pattern string
	lbu		a0, 0(s1)
	addiu	s1, 1
	beqz	a0, @@send_end
	nop
	jal		xp_sendbyte_rom
	nop
	b		@@send_loop
	nop
@@send_end:
	jal		xp_sendbyte_rom			; Send zero terminator
	move	a0, r0
	
	jal		xp_readbyte_rom			; Get error code
	nop
	
	sw		v0, IOB_errno(s0)
	bnez	v0, @@exit
	move	v0, r0
	
	lw		a2, 12(sp)
	nop
	sw		r0, DIRENT_next(a2)
	sw		r0, DIRENT_head(a2)
	sw		r0, DIRENT_sys(a2)
	
	jal		xp_readreg_rom			; Get file size
	nop
	
	lw		a2, 12(sp)
	nop
	sw		v0, DIRENT_size(a2)
	jal		xp_readbyte_rom
	nop
	sw		v0, DIRENT_attr(a2)
	
@@read_loop:						; Get directory entry name
	jal		xp_readbyte_rom
	nop
	beqz	v0, @@read_end
	sb		v0, 0(a2)
	b		@@read_loop
	addiu	a2, 1
	
@@read_end:

	lw		v0, 12(sp)
	nop
	lbu		v1, 0(v0)
	nop
	bnez	v1, @@exit
	nop	
	move	v0, r0
	
@@exit:
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	jr		ra
	addiu	sp, 16


xp_pcdrv_nfile:
	addiu	sp, -12
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)

	move	s0, a0
	move	s1, a1
	
	jal		xp_sendbyte_rom			; Send next file command
	addiu	a0, r0, 0x26
	
	jal		xp_readbyte_rom			; Get error code
	nop
	
	sw		v0, IOB_errno(s0)
	bnez	v0, @@exit
	move	v0, r0
	
	sw		r0, DIRENT_next(s1)
	sw		r0, DIRENT_head(s1)
	sw		r0, DIRENT_sys(s1)
	
	jal		xp_readreg_rom			; Get file size
	nop
	sw		v0, DIRENT_size(s1)		; Write to direntry
	
	jal		xp_readbyte_rom			; Get attribute
	nop
	sw		v0, DIRENT_attr(s1)		; Store to direntry
	
	move	a0, s1					; Get file name string
@@read_loop:
	jal		xp_readbyte_rom
	nop
	beqz	v0, @@read_end
	sb		v0, 0(a0)
	b		@@read_loop
	addiu	a0, 1
@@read_end:

	lbu		v1, 0(s1)
	nop
	bnez	v1, @@exit
	move	v0, s1
	
	move	v0, r0
	
@@exit:
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	jr		ra
	addiu	sp, 12


xp_pcdrv_chdir:
	
	addiu	sp, -12
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	
	move	s0, a0
	move	s1, a1
	
	jal		xp_sendbyte_rom			; Send chdir command
	addiu	a0, r0, 0x24
	
@@send_loop:						; Send directory path
	lbu		a0, 0(s1)
	addiu	s1, 1
	beqz	a0, @@send_done
	nop
	jal		xp_sendbyte_rom
	nop
	b		@@send_loop
	nop
@@send_done:
	jal		xp_sendbyte_rom
	move	a0, r0

	jal		xp_readbyte_rom			; Get error code
	nop
	
	sw		v0, IOB_errno(s0)
	beqz	v0, @@exit
	move	v0, r0
	
	addiu	v0, r0, 1
	
@@exit:
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	jr		ra
	addiu	sp, 12


xp_pcdrv_init:
	jr		ra
	nop
	
	
xp_readbytes_rom:
	
	addiu	sp, -8
	sw		ra, 0(sp)
	
@@read_loop:
	sw		a0, 4(sp)				; Save destination address
	jal		xp_readbyte_rom			; Read a byte
	nop
	lw		a0, 4(sp)				; Restore destination address
	addiu	a1, -1					; Decrement bytes to read
	sb		v0, 0(a0)				; Store read byte to destination address
	bgtz	a1, @@read_loop			; Loop
	addiu	a0, 1					; Increment for next byte
	
	lw		ra, 0(sp)
	addiu	sp, 8
	jr		ra
	nop
	
xp_readreg_rom:
	addiu	sp, -8
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	move	s0, r0
	move	a3, r0
@@read_loop:
	jal		xp_readbyte_rom
	nop
	sll		v1, s0, 3				; Equivalent to multiply by 8
	sllv	v0, v1
	addi	s0, 1
	blt		s0, 4, @@read_loop
	or		a3, v0
	move	v0, a3
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	jr		ra
	addiu	sp, 8
	
	
xp_sendbytes_rom:
	
	addiu	sp, -12
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	sw		s1, 8(sp)
	
	move	s0, a0
	move	s1, a1
	
@@send_loop:
	jal		xp_sendbyte_rom
	lbu		a0, 0(s0)
	addiu	s1, -1
	bgtz	s1, @@send_loop
	addiu	s0, 1
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	lw		s1, 8(sp)
	jr		ra
	addiu	sp, 12
	
	
xp_sendreg_rom:
	addiu	sp, -8
	sw		ra, 0(sp)
	sw		s0, 4(sp)
	
	move	a3, a0
	li		s0, 4
@@send_loop:
	jal		xp_sendbyte_rom
	andi	a0, a3, 0xFF
	addi	s0, -1
	bgtz	s0, @@send_loop
	srl		a3, 8
	
	lw		ra, 0(sp)
	lw		s0, 4(sp)
	jr		ra
	addiu	sp, 8
	
	
; PCDRV IOB block
xp_pcdrv_iob:
	dw	xp_pcdrv_name	; Device name
	dw	0x10			; Device flags
	dw	0x0				; Smallest logical block size
	dw	xp_pcdrv_desc	; Device description/long name
	dw	xp_pcdrv_init	; Init
	dw	xp_pcdrv_open	; Open
	dw	0				; in_out (only for TTY)
	dw	xp_pcdrv_close	; Close
	dw	0				; Ioctl
	dw	xp_pcdrv_read	; Read
	dw	xp_pcdrv_write	; Write
	dw	0				; Erase
	dw	0				; Undelete
	dw	xp_pcdrv_ffile	; First file
	dw	xp_pcdrv_nfile	; Next file
	dw	0				; Format
	dw	xp_pcdrv_chdir	; Chdir
	dw	0				; Rename
	dw	0				; Remove
	dw	0				; Test device
	
xp_pcdrv_name:
.asciiz "pcdrv"
xp_pcdrv_desc:
.asciiz "n00bROM PCDRV for Xplorer"

.align 4
