; Serial I/O ports
SIO_TXRX_REG	equ		0x1050
SIO_MODE_REG	equ		0x1058
SIO_CTRL_REG	equ		0x105A
SIO_BAUD_REG	equ		0x105E
SIO_STAT_REG	equ		0x1054
SIO_TXRX_REG_A	equ		0x1F801050
SIO_MODE_REG_A	equ		0x1F801058
SIO_CTRL_REG_A	equ		0x1F80105A
SIO_BAUD_REG_A	equ		0x1F80105E
SIO_STAT_REG_A	equ		0x1F801054
SIO_BAUD		equ		0x12

; Xplorer I/O ports
XP_IOBASE	equ		0x1F06
XP_sw		equ		0x0000
XP_data		equ		0x0001
XP_latch	equ		0x0001
XP_pcshake	equ		0x0002

; IRQ and DMA registers
ISTAT		equ		0x1070
IMASK		equ		0x1074
DPCR		equ 	0x10F0
DICR		equ 	0x10F4

; CD-ROM registers
CD_REG0 	equ 0x1800
CD_REG1 	equ 0x1801
CD_REG2 	equ 0x1802
CD_REG3 	equ 0x1803
COM_DELAY	equ	0x1020
