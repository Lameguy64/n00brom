#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#include "xplorer.h"

static int error_count;

void xp_ClearPort(int pport_fd)
{
	struct ppdev_frob_struct frob;
	int val = 0;
	
	// Turn off select line
	frob.mask	= PARPORT_CONTROL_SELECT;
	frob.val	= PARPORT_CONTROL_SELECT;
	
	// Clear data output
	ioctl(pport_fd, PPFCONTROL, &frob.val);
	ioctl(pport_fd, PPFCONTROL, &frob.val);
	ioctl(pport_fd, PPWDATA, &val);
	
	// Wait for ack line to become low
	/*
	while( xp_ReadPending(pport_fd) )
	{
		usleep(100);
	}
	*/
	
	error_count = 0;
}

int xp_ReadPending(int pport_fd)
{
	int status;
	ioctl(pport_fd, PPRSTATUS, &status);
	return(status & PARPORT_STATUS_ACK);
}

int xp_SendByte(int pport_fd, int byte)
{	
	struct ppdev_frob_struct frob;
	int timeout,status;
	
	// Send the data and put select high
	frob.mask	= PARPORT_CONTROL_SELECT;
	frob.val	= 0;
	ioctl(pport_fd, PPWDATA, &byte);
	ioctl(pport_fd, PPFCONTROL, &frob);
	
	// Wait until ack signal goes high
	status = 0;
	timeout = 0;
	do
	{
		if( timeout > 100000 )
		{
			frob.val = PARPORT_CONTROL_SELECT;
			ioctl(pport_fd, PPFCONTROL, &frob);
			ioctl(pport_fd, PPFCONTROL, &frob);
			return 1;
		}
		ioctl(pport_fd, PPRSTATUS, &status);
		timeout++;
	} while(!(status & PARPORT_STATUS_ACK));
	
	// Turn off select line
	frob.val	= PARPORT_CONTROL_SELECT;
	ioctl(pport_fd, PPFCONTROL, &frob);
	
	// Wait until ack signal goes low
	status = PARPORT_STATUS_ACK;
	timeout = 0;
	do
	{
		if( timeout > 100000 )
		{
			frob.val = PARPORT_CONTROL_SELECT;
			ioctl(pport_fd, PPFCONTROL, &frob);
			ioctl(pport_fd, PPFCONTROL, &frob);
			return 2;
		}
		ioctl(pport_fd, PPRSTATUS, &status);
		timeout++;
	} while((status & PARPORT_STATUS_ACK));
	
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
	struct ppdev_frob_struct frob;
	int i,p,byte,timeout,status,parity_fail;

retry:

	// Wait until ack signal goes high
	status = 0;
	timeout = 0;
	do
	{
		if( timeout > 10000 )
		{
			frob.val = PARPORT_CONTROL_SELECT;
			ioctl(pport_fd, PPFCONTROL, &frob);
			ioctl(pport_fd, PPFCONTROL, &frob);
			return -1;
		}
		ioctl(pport_fd, PPRSTATUS, &status);
		timeout++;
	} while(!(status & PARPORT_STATUS_ACK));
	
	// Get first three bits
	byte = ((status & PARPORT_STATUS_SELECT)>0)|
		(((status & PARPORT_STATUS_PAPEROUT)>0)<<1)|
		(((status & PARPORT_STATUS_BUSY)==0)<<2);
	
	// Put select high
	frob.mask	= PARPORT_CONTROL_SELECT;
	frob.val	= 0;
	ioctl(pport_fd, PPFCONTROL, &frob);
	
	// Wait until ack signal goes low
	status = 0;
	timeout = 0;
	do
	{
		if( timeout > 10000 )
		{
			frob.val  = PARPORT_CONTROL_SELECT;
			ioctl(pport_fd, PPFCONTROL, &frob);
			ioctl(pport_fd, PPFCONTROL, &frob);
			return -1;
		}
		ioctl(pport_fd, PPRSTATUS, &status);
		timeout++;
	} while((status & PARPORT_STATUS_ACK));
	
	// Get next three bits
	byte |= (((status & PARPORT_STATUS_SELECT)>0)<<3)|
		(((status & PARPORT_STATUS_PAPEROUT)>0)<<4)|
		(((status & PARPORT_STATUS_BUSY)==0)<<5);
		
	// Put select low
	frob.mask	= PARPORT_CONTROL_SELECT;
	frob.val	= PARPORT_CONTROL_SELECT;
	ioctl(pport_fd, PPFCONTROL, &frob);
	
	// Wait until ack signal goes high
	status = 0;
	timeout = 0;
	do
	{
		if( timeout > 10000 )
		{
			frob.val = PARPORT_CONTROL_SELECT;
			ioctl(pport_fd, PPFCONTROL, &frob);
			ioctl(pport_fd, PPFCONTROL, &frob);
			return -2;
		}
		ioctl(pport_fd, PPRSTATUS, &status);
		timeout++;
	} while(!(status & PARPORT_STATUS_ACK));
	
	// Get last two bits
	byte |= (((status & PARPORT_STATUS_SELECT)>0)<<6)|
		(((status & PARPORT_STATUS_PAPEROUT)>0)<<7);
		
	// Compute parity
	p = 0;
	for( i=0; i<8; i++ )
		p ^= ((byte>>i)&0x1);
	
	// Set data output if parity error occured or not
	parity_fail = 0;
	if( ((status & PARPORT_STATUS_BUSY)==0) != p )
	{
		i = 0xff;
		parity_fail = 1;
		error_count++;
	}
	else
	{
		i = 0;
	}
	ioctl(pport_fd, PPWDATA, &i);
	
	// Put select high
	frob.mask	= PARPORT_CONTROL_SELECT;
	frob.val	= 0;
	ioctl(pport_fd, PPFCONTROL, &frob);
	
	// Wait until ack signal goes low
	status = 0;
	timeout = 0;
	do
	{
		if( timeout > 10000 )
		{
			frob.val = PARPORT_CONTROL_SELECT;
			ioctl(pport_fd, PPFCONTROL, &frob);
			ioctl(pport_fd, PPFCONTROL, &frob);
			return -3;
		}
		ioctl(pport_fd, PPRSTATUS, &status);
		timeout++;
	} while((status & PARPORT_STATUS_ACK));
	
	// Put select low
	frob.mask	= PARPORT_CONTROL_SELECT;
	frob.val	= PARPORT_CONTROL_SELECT;
	ioctl(pport_fd, PPFCONTROL, &frob);
	
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
