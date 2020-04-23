; Struct for HookEntryInt
INT_pc		equ		0
INT_sp		equ		4
INT_fp		equ		8
INT_s0		equ		12
INT_s1		equ		16
INT_s2		equ		20
INT_s3		equ		24
INT_s4		equ		28
INT_s5		equ		32
INT_s6		equ		36
INT_s7		equ		40
INT_gp		equ		44
INT_size	equ		48

; File control block for TTY and PCDRV stuff
FCB_status		equ	0
FCB_diskid		equ	4
FCB_taddr		equ	8
FCB_tlen		equ	12
FCB_filepos		equ	16
FCB_flags		equ	20
FCB_lasterr		equ	24
FCB_dcb			equ	28
FCB_filesize	equ	32
FCB_lba			equ	36
FCB_fcbnum		equ	40

; PS-EXE parameters
EXEC_pc0	equ		0
EXEC_gp0	equ		4
EXEC_taddr	equ		8
EXEC_tsize	equ		12
EXEC_daddr	equ		16
EXEC_dsize	equ		20
EXEC_baddr	equ		24
EXEC_bsize	equ		28
EXEC_saddr	equ		32
EXEC_ssize	equ		36
EXEC_sp		equ		40
EXEC_fp		equ		44
EXEC_ra		equ		48
EXEC_ret	equ		52
EXEC_base	equ		56
EXEC_crc32	equ		60
EXEC_flags	equ		64
EXEC_len	equ		68

; BIN download structure
BIN_size	equ		0
BIN_addr	equ		4
BIN_crc32	equ		8
BIN_len		equ		12
