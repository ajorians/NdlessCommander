CC = nspire-gcc
LD = nspire-ld
GENZEHN = genzehn

SOURCES = nc.c
EXE = NC
OBJECTS = $(SOURCES:.c=.o)

OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -Wall -Wextra -Ofast -DALLOW_DELETE
#CFLAGS = -Wall -Wextra -Ofast
LDFLAGS =
LIBS = -lSDL
DISTDIR = .

all: $(EXE).prg.tns

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE).elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $(DISTDIR)/$@

$(EXE).tns: $(EXE).elf
	$(GENZEHN) --input $(DISTDIR)/$^ --output $(DISTDIR)/$@ $(ZEHNFLAGS)

$(EXE).prg.tns: $(EXE).tns
	make-prg $(DISTDIR)/$^ $(DISTDIR)/$@

clean:
	rm -f *.o *.elf
	rm -f $(DISTDIR)/$(EXE).tns
	rm -f $(DISTDIR)/$(EXE).elf
	rm -f $(DISTDIR)/$(EXE).prg.tns
