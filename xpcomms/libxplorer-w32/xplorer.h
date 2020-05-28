#ifndef _XPLORER_H
#define _XPLORER_H

#ifdef __cplusplus
extern "C" {
#endif

void xp_ClearPort(int pport_fd);
int xp_ReadPending(int pport_fd);

int xp_SendByte(int pport_fd, int byte);
int xp_SendBytes(int pport_fd, unsigned char *bytes, int len);
int xp_ReadByte(int pport_fd);
int xp_ReadBytes(int pport_fd, unsigned char *bytes, int len);

#ifdef __cplusplus
}
#endif

#endif // _XPLORER_H
