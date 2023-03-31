DEVICE			= atmega48p
F_CPU			= 8000000

# F in Hz
FUSE_L			= 0xE2
FUSE_H			= 0xDF
AVRDUDE			= avrdude -c avrisp2 -p $(DEVICE)

CFLAGS			= -std=c99 -Wall -g -Os -mmcu=${DEVICE} -DF_CPU=${F_CPU} -I.
OBJECTS			= main.o

TARGET			= main
SRC				= main.c

CC				= avr-gcc
OBJCOPY			= avr-objcopy

# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"

# rule for programming fuse bits:
fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

all:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRC}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex

flash:
	$(AVRDUDE) -U flash:w:${TARGET}.hex:i -P usb

clean:
	rm -f *.bin *.hex

size:
	avr-size --mcu=atmega48p main.bin -C

