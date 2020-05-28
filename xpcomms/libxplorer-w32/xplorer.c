#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <inpout32.h>
#include "xplorer.h"

#define TIMEOUT_COUNT		10000

#define LPT_BASE			0x378
#define LPT_STAT			0x1
#define LPT_CTRL			0x2

#define LPT_STAT_ERROR		0x8
#define LPT_STAT_SELECT		0x10
#define LPT_STAT_PAPEROUT	0x20
#define LPT_STAT_ACK		0x40
#define LPT_STAT_BUSY		0x80

#define LPT_CTRL_STROBE		0x1
#define LPT_CTRL_AUTOFD		0x2
#define LPT_CTRL_INIT		0x4
#define LPT_CTRL_SELECT		0x8

static int error_count;

void xp_ClearPort(int pport_fd)
{
	// Clear data and control registers
	Out32(pport_fd, 0x0);
	Out32(pport_fd+LPT_CTRL, 0);
	
	error_count = 0;
}

int xp_ReadPending(int pport_fd)
{
	int status;
	
	status = Inp32(pport_fd+LPT_STAT);
	
	return (status & LPT_STAT_ACK);
}

int xp_SendByte(int pport_fd, int byte)
{	
	int timeout,status;
	
	// Set data and put select low
	Out32(pport_fd, byte);
	Out32(pport_fd+LPT_CTRL, LPT_CTRL_SELECT);
	
	// Wait for ACK signal to go high
	timeout = 0;
	do
	{
		if( timeout > TIMEOUT_COUNT )
		{
			Out32(pport_fd+LPT_CTRL, 0);
			return 1;
		}
		status = Inp32(pport_fd+LPT_STAT);
		timeout++;
	} while(!(status & LPT_STAT_ACK));
	
	// Bring select high and wait for ACK to go low
	Out32(pport_fd+LPT_CTRL, 0);
	
	timeout = 0;
	do
	{
		if( timeout > TIMEOUT_COUNT )
		{
			return 1;
		}
		status = Inp32(pport_fd+LPT_STAT);
		timeout++;
	} while((status & LPT_STAT_ACK));
	
	return 0;
}

int xp_SendBytes(int pport_fd, unsigned char *bytes, int len)
{
	int i;
	
	for( i=0; i<len; i++ )
	{
		if( xp_SendByte(pport_fd, bytes[i]) )
		{
			return -1;
		}
	}
	
	return 0;
}



int xp_ReadByte(int pport_fd)
{
	int timeout,status,byte,parity_fail,i,p;
	
retry:

	// Wait until ack signal goes high
	timeout = 0;
	do
	{
		if( timeout > TIMEOUT_COUNT )
		{
			return -1;
		}
		status = Inp32(pport_fd+LPT_STAT);
		timeout++;
	} while(!(status & LPT_STAT_ACK));
	
	// Get first three bits
	byte = ((status & LPT_STAT_SELECT)>0)|
		(((status & LPT_STAT_PAPEROUT)>0)<<1)|
		(((status & LPT_STAT_BUSY)==0)<<2);
	
	// Put select low
	Out32(pport_fd+LPT_CTRL, LPT_CTRL_SELECT);
	
	// Wait until ack goes low
	timeout = 0;
	do
	{
		if( timeout > TIMEOUT_COUNT )
		{
			Out32(pport_fd+LPT_CTRL, 0);
			return -1;
		}
		status = Inp32(pport_fd+LPT_STAT);
		timeout++;
	} while((status & LPT_STAT_ACK));
	
	// Get next three bits
	byte |= (((status & LPT_STAT_SELECT)>0)<<3)|
		(((status & LPT_STAT_PAPEROUT)>0)<<4)|
		(((status & LPT_STAT_BUSY)==0)<<5);
	
	// Put select high
	Out32(pport_fd+LPT_CTRL, 0);
	
	// Wait until ack signal goes high
	timeout = 0;
	do
	{
		if( timeout > TIMEOUT_COUNT )
		{
			return -1;
		}
		status = Inp32(pport_fd+LPT_STAT);
		timeout++;
	} while(!(status & LPT_STAT_ACK));
	
	// Get last two bits
	byte |= (((status & LPT_STAT_SELECT)>0)<<6)|
		(((status & LPT_STAT_PAPEROUT)>0)<<7);
		
	// Compute parity
	p = 0;
	for( i=0; i<8; i++ )
		p ^= ((byte>>i)&0x1);
	
	// Set data output if parity error occured or not
	parity_fail = 0;
	if( ((status & LPT_STAT_BUSY)==0) != p )
	{
		i = 0xff;
		parity_fail = 1;
		error_count++;
	}
	else
	{
		i = 0;
	}
	Out32(pport_fd, i);
	
	// Put select low
	Out32(pport_fd+LPT_CTRL, LPT_CTRL_SELECT);
	
	// Wait until ack signal goes low
	timeout = 0;
	do
	{
		if( timeout > TIMEOUT_COUNT )
		{
			Out32(pport_fd+LPT_CTRL, 0);
			return -3;
		}
		status = Inp32(pport_fd+LPT_STAT);
		timeout++;
	} while((status & LPT_STAT_ACK));
	
	// Put select high
	Out32(pport_fd+LPT_CTRL, 0);
	
	if( parity_fail )
		goto retry;
	
	return byte;
}

int xp_ReadBytes(int pport_fd, unsigned char *bytes, int len)
{
	int i,j;
	
	for( i=0; i<len; i++ )
	{
		if( (j = xp_ReadByte(pport_fd)) < 0 )
		{
			return -1;
		}
		bytes[i] = j;
	}
	
	return 0;
}
