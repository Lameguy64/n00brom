TARGET	= xpcomms

CFILES	= xpcomms.c pcdrv.c

# Library options for Win32
ifeq ($(OS),Windows_NT)

COMMLIB	= libxplorer-w32

INCLUDE = -Ilibxplorer-w32 -I/c/inpout32
LIBS	= -Wl,--enable-stdcall-fixup /c/inpout32/inpout32.dll

# Library options for *nixes
else

COMMLIB	= libxplorer

endif

INCLUDE += -I$(COMMLIB)
LIBS	+= -L$(COMMLIB) -lxplorer

OFILES	= $(addprefix build/,$(CFILES:.c=.o))

CFLAGS	= -O2

CC		= gcc

all: $(OFILES)
	make -C $(COMMLIB)
	$(CC) $(CFLAGS) $(OFILES) $(LIBS) -o $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
clean:
	make -C $(COMMLIB) clean
	rm -Rf build
