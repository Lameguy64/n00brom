TARGET	= libxplorer.a

CFILES	= xplorer.c

INSTALL = /usr


OFILES	= $(addprefix build/,$(CFILES:.c=.o))

CFLAGS	= -g -O2


INCLUDE	= -I/c/inpout32


CC		= gcc
AR		= ar
RANLIB	= ranlib


all: $(TARGET)

$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)
	
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

install:
	cp xplorer.h $(INSTALL)/include/
	cp libxplorer.a $(INSTALL)/lib/
	
clean:
	rm -Rf build $(TARGET)
