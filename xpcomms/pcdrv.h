#ifndef _PCDRV_H
#define _PCDRV_H

// File modes
#define PCDRV_FREAD		0x0001
#define PCDRV_FWRITE	0x0002
#define PCDRV_FAPPEND	0x0100
#define PCDRV_FCREATE	0x0200
#define PCDRV_FTRUNC	0x0400

// Error codes
#define PCDRV_EPERM		1
#define PCDRV_ENOENT	2
#define PCDRV_EIO		5
#define PCDRV_ENOTDIR	20
#define PCDRV_EMFILE	24
#define PCDRV_ENOSPC	28
#define PCDRV_EROFS		30
#define PCDRV_EHFULL	253
#define PCDRV_ECSUM		254
#define PCDRV_EUNDEF	255

#define PCDRV_MAXFILES	16

#ifdef __cplusplus
extern "C" {
#endif

void pcdrv_init(void);
void pcdrv_deinit(void);

int pcdrv_parse(int lpt_fd, int cmd);

#ifdef __cplusplus
}
#endif

#endif // _PCDRV_H
