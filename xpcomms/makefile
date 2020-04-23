TARGET	= xpcomms

CFILES	= xpcomms.c pcdrv.c

INCLUDE = -Ilibxplorer
LIBS	= -Llibxplorer -lxplorer


OFILES	= $(addprefix build/,$(CFILES:.c=.o))

CFLAGS	= -O2

CC		= gcc

all: $(OFILES)
	make -C libxplorer
	$(CC) $(CFLAGS) $(OFILES) $(LIBS) -o $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
