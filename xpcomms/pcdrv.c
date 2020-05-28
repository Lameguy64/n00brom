#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#ifndef __WIN32
#include <fnmatch.h>
#else
#include <windows.h>
#include <winerror.h>
#endif
#include <xplorer.h>
#include "pcdrv.h"


static FILE *pcdrv_fd[PCDRV_MAXFILES];

#ifndef __WIN32
static DIR *pcdrv_dir;
static char *pcdrv_dir_pattern;
static char *pcdrv_dir_base;
#else
static HANDLE hFind;
#endif

void pcdrv_init(void)
{
	int i;
	
	for( i=0; i<PCDRV_MAXFILES; i++ )
		pcdrv_fd[i] = NULL;

#ifndef __WIN32	
	pcdrv_dir = NULL;
	pcdrv_dir_pattern = NULL;
	pcdrv_dir_base = NULL;
#else
	hFind = INVALID_HANDLE_VALUE;
#endif
}

void pcdrv_deinit(void)
{
	int i;
	
	// Close any open handles
	for( i=0; i<PCDRV_MAXFILES; i++ )
	{
		if( pcdrv_fd[i] )
			fclose(pcdrv_fd[i]);
	}
	
	// Close open directory query
#ifndef __WIN32
	if( pcdrv_dir )
		closedir(pcdrv_dir);
#else
	if( hFind != INVALID_HANDLE_VALUE )
		FindClose(hFind);
#endif
}

void pcdrv_open(int lpt_fd)
{
	int i,j,nl,handle;
	char *filename;
	const char *fmode;
	struct stat fs;
	
	// Get open flags
	i = 0;
	if( xp_ReadBytes(lpt_fd, (unsigned char*)&i, 2) < 0 )
	{
		printf("PCDRV open: Timeout in receiving mode flags.\n");
		return;
	}
	
	// Determine file mode by file flags
	fmode = NULL;
	if( (i & PCDRV_FREAD) && (i & PCDRV_FWRITE) )
	{
		if( (i & PCDRV_FAPPEND) )
		{
			fmode = "ab+";
		}
		else
		{
			if( (i & PCDRV_FCREATE ) || (i & PCDRV_FTRUNC ) )
				fmode = "wb+";
			else
				fmode = "rb+";
		}
	}
	else if( (i & PCDRV_FREAD) )
	{
		fmode = "r";
	}
	else if( (i & PCDRV_FWRITE) )
	{
		if( (i & PCDRV_FCREATE ) || (i & PCDRV_FTRUNC ) )
			fmode = "wb";
		else
			fmode = "ab";
	}
	
	if( fmode == NULL )
	{
		printf("PCDRV open: Cannot determine file mode.\n");
		return;
	}
	
	// Get file name
	filename = (char*)malloc(1);
	filename[0] = 0;
	nl = 0;
	while( i = xp_ReadByte(lpt_fd) )
	{
		if( i < 0 )
		{
			printf("PCDRV open: Timeout receiving file name.\n");
			free(filename);
			return;
		}
		filename = (char*)realloc(filename, nl+2);
		filename[nl] = i;
		nl++;
	}
	filename[nl] = 0;
	
	// Find a free handle
	for( handle=0; (pcdrv_fd[handle]!=0)&&(handle<PCDRV_MAXFILES); handle++ );
	
	if( handle == PCDRV_MAXFILES )
	{
		printf("PCDRV open: File handles exhausted.\n");
		xp_SendByte(lpt_fd, PCDRV_EMFILE);
		free(filename);
		return;
	}
	
	// Open file
	pcdrv_fd[handle] = fopen(filename, fmode);
	if( !pcdrv_fd[handle] )
	{
		switch(errno)
		{
		case EPERM:
			printf("PCDRV open: Permission denied on file: %s\n", filename);
			xp_SendByte(lpt_fd, PCDRV_EPERM);
			break;
		case ENOENT:
			printf("PCDRV open: No such file or directory: %s\n", filename);
			xp_SendByte(lpt_fd, PCDRV_ENOENT);
			break;
		case EIO:
			printf("PCDRV open: I/O error opening file: %s\n", filename);
			xp_SendByte(lpt_fd, PCDRV_EIO);
			break;
		default:
			printf("PCDRV open: Undefined error (%d) opening file: %s\n", 
				errno, filename);
			xp_SendByte(lpt_fd, PCDRV_EUNDEF);
			break;
		}
		free(filename);
		return;
	}
	
	// Send OK error code and file handle
	xp_SendByte(lpt_fd, 0);
	xp_SendByte(lpt_fd, handle);
	
	// Send file size
	stat(filename, &fs);
	xp_SendBytes(lpt_fd, (unsigned char*)&fs.st_size, 4);
	
	free(filename);
}

void pcdrv_close(int lpt_fd)
{
	int i,handle;
	
	// Get file handle
	handle = xp_ReadByte(lpt_fd);
	if( handle < 0 )
	{
		printf("PCDRV close: Timeout in receiving file handle.\n");
		return;
	}
	
	if( handle > PCDRV_MAXFILES )
	{
		printf("PCDRV close: File handle (%d) exceeds max handles.\n", handle);
		return;
	}
	
	if( pcdrv_fd[handle] == NULL )
	{
		printf("PCDRV close: Specified handle (%d) is not a open handle.\n",
			handle);
		return;
	}
	
	fclose(pcdrv_fd[handle]);
	pcdrv_fd[handle] = NULL;
}

int readbytes(int lpt_fd, unsigned char *buff, int len)
{
	int i,j;
	for( i=0; i<len; i++ )
	{
		j = xp_ReadByte(lpt_fd);
		if( j < 0 )
			return -1;
		buff[i] = j;
	}
	
	return len;
}

void pcdrv_read(int lpt_fd)
{
	int i,j,k;
	int handle,len,err,csum;
	unsigned char *buff;
	
	// Get handle
	handle = xp_ReadByte(lpt_fd);
	if( handle < 0 )
	{
		printf("PCDRV read: Timeout receiving file handle.\n");
		return;
	}
	if( handle > PCDRV_MAXFILES )
	{
		printf("PCDRV read: File handle (%d) exceeds max handles.\n", handle);
		return;
	}
	if( pcdrv_fd[handle] == NULL )
	{
		printf("PCDRV read: File handle not open.\n");
		return;
	}
	
	// Get read length
	if( readbytes(lpt_fd, (unsigned char*)&len, 4) < 0 )
	{
		printf("PCDRV read: Timeout in receiving read length.\n");
		return;
	}
	
	// Read from file
	buff = (unsigned char*)malloc(len);
	i = fread(buff, 1, len, pcdrv_fd[handle]);
	
	err = 0;
	if( i != len )
	{
		if( !feof(pcdrv_fd[handle]) )
		{
			switch( errno )
			{
			case EIO:
				err = PCDRV_EIO;
				break;
			default:
				err = PCDRV_EUNDEF;
			}
		}
	}
	
	// Send error code
	xp_SendByte(lpt_fd, err);

	// Just in case fread returns negative
	if( i < 0 )
		i = 0;

	// Send read result
	xp_SendBytes(lpt_fd, (unsigned char*)&i, 4);
	
	if( i > 0 )
	{
		len = i;
		while(1)
		{
			// Send bytes
			csum = 0;
			for( i=0; i<len; i++ )
			{
				if( xp_SendByte(lpt_fd, buff[i]) )
				{
					printf("PCDRV read: Timeout sending read data.\n");
					free(buff);
					return;
				}
				
				// Compute CRC8 checksum
				j = buff[i];
				for( k=0; k<8; k++ )
				{
					int sum = (csum ^ j) & 0x1;
					csum >>= 1;
					if( sum )
						csum ^= 0x8C;
					j >>= 1;
				}
			}
						
			// Send checksum
			if( xp_SendByte(lpt_fd, csum) )
			{
				printf("PCDRV read: Timeout sending checksum byte.\n");
				free(buff);
				return;
			}
	
			// Receive checksum verify
			i = xp_ReadByte(lpt_fd);
			if( i < 0 )
			{
				printf("PCDRV read: Timeout on checksum verify..\n");
				free(buff);
				return;
			}
	
			// Checksum OK if return value is zero
			if( i == 0 )
				break;
			
			printf("PCDRV read: Checksum error, retrying...\n");
		}
	}
					
	// Send file offset
	i = ftell(pcdrv_fd[handle]);
	if( xp_SendBytes(lpt_fd, (unsigned char*)&i, 4) )
	{
		printf("PCDRV read: Timeout sending new file offset.\n");
	}

	free(buff);
}

void pcdrv_write(int lpt_fd)
{
	int i,j,k;
	int handle,pos,len,csum;
	unsigned char *buff;
	
	// Get handle
	handle = xp_ReadByte(lpt_fd);
	if( handle < 0 )
	{
		printf("PCDRV write: Timeout receiving file handle.\n");
		return;
	}
	if( handle > PCDRV_MAXFILES )
	{
		printf("PCDRV write: File handle (%d) exceeds max handles.\n", handle);
		return;
	}
	if( pcdrv_fd[handle] == NULL )
	{
		printf("PCDRV write: File handle not open.\n");
		return;
	}
	
	// Get file position
	if( readbytes(lpt_fd, (unsigned char*)&pos, 4) < 0 )
	{
		printf("PCDRV write: Timeout receiving file position.\n");
		return;
	}
	
	// Get write length
	if( readbytes(lpt_fd, (unsigned char*)&len, 4) < 0 )
	{
		printf("PCDRV write: Timeout receiving write length.\n");
		return;
	}

	// Get write bytes
	buff = (unsigned char*)malloc(len);

retry:

	csum = 0;
	for( i=0; i<len; i++ )
	{
		j = xp_ReadByte(lpt_fd);
		if( j < 0 )
		{
			printf("PCDRV write: Timeout receiving write data %d/%d.\n", 
				i, len);
			free(buff);
			return;
		}
		buff[i] = j;
		
		// Compute CRC8 checksum
		for( k=0; k<8; k++ )
		{
			int sum = (csum ^ j) & 0x1;
			csum >>= 1;
			if( sum )
				csum ^= 0x8C;
			j >>= 1;
		}
		
	}
	
	// Get checksumfg
	i = xp_ReadByte(lpt_fd);
	if( i != csum )
	{
		printf("PCDRV write: Checksum error, retrying.\n");
		
		// Do transfer retry
#ifndef __WIN32
		usleep(1000);
#else
		Sleep(1);
#endif
		xp_SendByte(lpt_fd, PCDRV_ECSUM);
		
		goto retry;
	}
	
	// Write data
	fseek(pcdrv_fd[handle], pos, SEEK_SET);
	i = fwrite(buff, 1, len, pcdrv_fd[handle]);
	free(buff);

	// Error
	j = 0;
	if( i != len )
	{
		printf("PCDRV write: Host write error warning.\n");
		switch(errno)
		{
		case EIO:
			j = PCDRV_EIO;
			break;
		case ENOSPC:
			j = PCDRV_ENOSPC;
			break;
		case EROFS:
			j = PCDRV_EROFS;
			break;
		default:
			j = PCDRV_EUNDEF;
		}
	}
					
	// Send error code
	xp_SendByte(lpt_fd, j);
	
	// Send bytes written
	xp_SendByte(lpt_fd, (i&0xFF));
	xp_SendByte(lpt_fd, (i>>8)&0xFF);
	xp_SendByte(lpt_fd, (i>>16)&0xFF);
	xp_SendByte(lpt_fd, (i>>24)&0xFF);
					
	// Send new file offset
	i = ftell(pcdrv_fd[handle]);
	xp_SendByte(lpt_fd, (i&0xFF));
	xp_SendByte(lpt_fd, (i>>8)&0xFF);
	xp_SendByte(lpt_fd, (i>>16)&0xFF);
	xp_SendByte(lpt_fd, (i>>24)&0xFF);
}

void pcdrv_firstfile(int lpt_fd)
{
#ifndef __WIN32
	struct stat fs;
	struct dirent *dir;
#else
	WIN32_FIND_DATA FindFileData;
#endif
	int i,j;
	char *dir_pattern;
	char *c;
	
	// Get directory pattern
	dir_pattern = (char*)malloc(1);
	dir_pattern[0] = 0;
	j = 0;
	while( i = xp_ReadByte(lpt_fd) )
	{
		if( i < 0 )
		{
			printf("PCDRV firstfile: Timeout receiving search pattern.\n");
			free(dir_pattern);
			return;
		}
		dir_pattern = (char*)realloc(dir_pattern, j+2);
		dir_pattern[j] = i;
		j++;
	}
	dir_pattern[j] = 0;
	
#ifndef __WIN32

	// Clear previously allocated directory strings
	if( pcdrv_dir_pattern )
		free(pcdrv_dir_pattern);
	pcdrv_dir_pattern = NULL;
	
	if( pcdrv_dir_base )
		free(pcdrv_dir_base);
	pcdrv_dir_base = NULL;
	
	// Get base directory name
	if( (c = strrchr(dir_pattern, '/')) )		// If directory path
	{
		pcdrv_dir_pattern = strdup(c+1);	// Extract file pattern
		pcdrv_dir_base = strndup(dir_pattern, (c-dir_pattern)+1);
	}
	else
	{
		pcdrv_dir_pattern = strdup(dir_pattern);
		pcdrv_dir_base = strdup("./");
	}
	
	free(dir_pattern);
	
	// Open directory
	printf("PCDRV firstfile: Opening directory %s...\n", pcdrv_dir_base);

	if( pcdrv_dir )
		closedir(pcdrv_dir);
	
	i = 0;
	if( !(pcdrv_dir = opendir(pcdrv_dir_base)) )
	{
		switch(errno)
		{
		case EPERM:
			printf("PCDRV firstfile: Permission denied.\n");
			i = PCDRV_EPERM;
			break;
		case ENOENT:
			printf("PCDRV firstfile: Directory does not exist.\n");
			i = PCDRV_ENOENT;
			break;
		case ENOTDIR:
			printf("PCDRV firstfile: Name is not a directory.\n");
			i = PCDRV_ENOTDIR;
			break;
		default:
			printf("PCDRV firstfile: Undefined error (%d).\n", errno);
			i = PCDRV_EUNDEF;
		}
	}
	
#else

	if( hFind != INVALID_HANDLE_VALUE )
		FindClose(hFind);
	
	i = 0;
	hFind = FindFirstFile(dir_pattern, &FindFileData);
	if( hFind == INVALID_HANDLE_VALUE )
	{
		if( GetLastError() == ERROR_FILE_NOT_FOUND )
		{
			xp_SendByte(lpt_fd, 0);
			xp_SendBytes(lpt_fd, (unsigned char*)&i, 4);
			xp_SendByte(lpt_fd, 0);
			xp_SendByte(lpt_fd, 0);
			return;
		}
		
		switch( GetLastError() )
		{
		case ERROR_ACCESS_DENIED:
			printf("PCDRV firstfile: Permission denied.\n");
			i = PCDRV_EPERM;
			break;
		case ERROR_PATH_NOT_FOUND:
			printf("PCDRV firstfile: Name is not a directory.\n");
			i = PCDRV_ENOENT;
			break;
		default:
			printf("PCDRV firstfile: FindFirstFile() failed (%d)\n", GetLastError());
			i = PCDRV_EUNDEF;
		}
	}
	
	free(dir_pattern);
	
#endif

	// Send error code
	xp_SendByte(lpt_fd, i);
	
#ifndef __WIN32
	
	if( !pcdrv_dir )
		return;

#else
	
	if( hFind == INVALID_HANDLE_VALUE )
		return;

#endif
		
	// Get dir entry
#ifndef __WIN32

	errno = 0;
	while( dir = readdir(pcdrv_dir) )
	{
		if( dir )
		{
			if( fnmatch( pcdrv_dir_pattern, dir->d_name, 0 ) == 0 )
			{
				c = (char*)malloc(strlen(pcdrv_dir_base)+strlen(dir->d_name)+1);
				
				strcpy(c, pcdrv_dir_base);
				strcat(c, dir->d_name);
				stat(c, &fs);
				
				free(c);
		
				xp_SendBytes(lpt_fd, (unsigned char*)&fs.st_size, 4);
				xp_SendByte(lpt_fd, dir->d_type);
		
				for( i=0; (dir->d_name[i])&&(i<19); i++ )
					xp_SendByte(lpt_fd, dir->d_name[i]);
			
				xp_SendByte(lpt_fd, 0);
			
				return;
			}
		}
	}
	
	// If no matching entry found
	i = 0;
	xp_SendBytes(lpt_fd, (unsigned char*)&i, 4);
	xp_SendByte(lpt_fd, 0);
	xp_SendByte(lpt_fd, 0);
	
#else
	
	xp_SendBytes(lpt_fd, (unsigned char*)&FindFileData.nFileSizeLow, 4);
	
	if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		xp_SendByte(lpt_fd, 16);
	else
		xp_SendByte(lpt_fd, 0);
	
	for( i=0; (FindFileData.cFileName[i])&&(i<19); i++ )
		xp_SendByte(lpt_fd, FindFileData.cFileName[i]);
	
	xp_SendByte(lpt_fd, 0);
	
#endif
	
}

void pcdrv_nextfile(int lpt_fd)
{
#ifndef __WIN32
	struct stat fs;
	struct dirent *dir;
#else
	WIN32_FIND_DATA FindFileData;
#endif
	int i,j;
	char *c;
	
#ifndef __WIN32
	if( !pcdrv_dir )
#else
	if( hFind == INVALID_HANDLE_VALUE )
#endif
	{
		i = 0;
		xp_SendByte(lpt_fd, PCDRV_ENOTDIR);
		printf("PCDRV nextfile: No firstfile issued prior.\n");
		return;
	}
	
#ifndef __WIN32

	// Send ok error code
	xp_SendByte(lpt_fd, 0);

	// Get dir entry
	while( dir = readdir(pcdrv_dir) )
	{
		if( dir )
		{
			if( fnmatch( pcdrv_dir_pattern, dir->d_name, 0 ) == 0 )
			{
				c = (char*)malloc(strlen(pcdrv_dir_base)+strlen(dir->d_name)+1);
				
				strcpy(c, pcdrv_dir_base);
				strcat(c, dir->d_name);
				stat(c, &fs);
				
				free(c);
		
				xp_SendBytes(lpt_fd, (unsigned char*)&fs.st_size, 4);
				xp_SendByte(lpt_fd, dir->d_type);
		
				for( i=0; (dir->d_name[i])&&(i<19); i++ )
					xp_SendByte(lpt_fd, dir->d_name[i]);
			
				xp_SendByte(lpt_fd, 0);
			
				return;
			}
		}
	}
	
	// If no matching entry found
	i = 0;
	xp_SendBytes(lpt_fd, (unsigned char*)&i, 4);
	xp_SendByte(lpt_fd, 0);
	xp_SendByte(lpt_fd, 0);
	
#else

	i = 0;
	if( !FindNextFile(hFind, &FindFileData) )
	{
		if( GetLastError() == ERROR_NO_MORE_FILES )
		{
			xp_SendByte(lpt_fd, 0);
			xp_SendBytes(lpt_fd, (unsigned char*)&i, 4);
			xp_SendByte(lpt_fd, 0);
			xp_SendByte(lpt_fd, 0);
			return;
		}
		
		switch( GetLastError() )
		{
		case ERROR_ACCESS_DENIED:
			printf("PCDRV firstfile: Permission denied.\n");
			i = PCDRV_EPERM;
			break;
		case ERROR_PATH_NOT_FOUND:
			printf("PCDRV firstfile: Name is not a directory.\n");
			i = PCDRV_ENOENT;
			break;
		default:
			printf("PCDRV firstfile: FindFirstFile() failed (%d)\n", GetLastError());
			i = PCDRV_EUNDEF;
		}
	}
	
	xp_SendByte(lpt_fd, i);
	
	xp_SendBytes(lpt_fd, (unsigned char*)&FindFileData.nFileSizeLow, 4);
	
	if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		xp_SendByte(lpt_fd, 16);
	else
		xp_SendByte(lpt_fd, 0);
	
	for( i=0; (FindFileData.cFileName[i])&&(i<19); i++ )
		xp_SendByte(lpt_fd, FindFileData.cFileName[i]);
	
	xp_SendByte(lpt_fd, 0);
	
#endif
}

void pcdrv_chdir(int lpt_fd)
{
	int i,j;
	char *dir_path;
	
	// Get directory pattern
	dir_path = (char*)malloc(1);
	dir_path[0] = 0;
	j = 0;
	while( i = xp_ReadByte(lpt_fd) )
	{
		if( i < 0 )
		{
			printf("PCDRV chdir: Timeout receiving path string.\n");
			free(dir_path);
			return;
		}
		dir_path = (char*)realloc(dir_path, j+2);
		dir_path[j] = i;
		j++;
	}
	dir_path[j] = 0;
	
	// Do chdir operation
	i = 0;
	if( chdir(dir_path) )
	{
		switch(errno)
		{
		case EPERM:
			printf("PCDRV chdir: Permission denied.\n");
			i = PCDRV_EPERM;
			break;
		case ENOENT:
			printf("PCDRV chdir: Directory does not exist.\n");
			i = PCDRV_ENOENT;
			break;
		case ENOTDIR:
			printf("PCDRV chdir: Name is not a directory.\n");
			i = PCDRV_ENOENT;
			break;
		default:
			printf("PCDRV chdir: Undefined error (%d).\n", errno);
			i = PCDRV_EUNDEF;
		}
	}
	
	xp_SendByte(lpt_fd, i);
}

void pcdrv_char(int lpt_fd)
{
	int i;
	
	do
	{
		i = xp_ReadByte(lpt_fd);
		if( i < 0 )
			return;
		
		putchar(i);
		fflush(stdout);
		
	} while( (i = xp_ReadByte(lpt_fd)) == 0x2F );
	
	if( i >= 0 )
	{
		pcdrv_parse(lpt_fd, i);
	}
}

int pcdrv_parse(int lpt_fd, int cmd)
{
	switch(cmd)
	{
	case 0x20:
		pcdrv_open(lpt_fd);
		return 1;
	case 0x21:
		pcdrv_close(lpt_fd);
		return 1;
	case 0x22:
		pcdrv_read(lpt_fd);
		return 1;
	case 0x23:
		pcdrv_write(lpt_fd);
		return 1;
	case 0x24:
		pcdrv_chdir(lpt_fd);
		return 1;
	case 0x25:
		pcdrv_firstfile(lpt_fd);
		return 1;
	case 0x26:
		pcdrv_nextfile(lpt_fd);
		return 1;
	case 0x2F:
		pcdrv_char(lpt_fd);
		return 1;
	}
	return 0;
}
