include $(ROOT)/usr/include/make/PRdefs

NUSYSINCDIR  = $(N64KITDIR)/nusys/include
NUSYSLIBDIR  = $(N64KITDIR)/nusys/lib

SRCDIR = src
INCDIR = include
ASSDIR = assets
OBJDIR = build

TARGET = mine64

LIB = $(ROOT)/usr/lib
CC  = gcc
LD  = ld
MAKEROM = mild
MAKEMASK = makemask

LCDEFS =	-DNU_DEBUG -DF3DEX_GBI_2
LCINCS =	-I. -I$(NUSYSINCDIR) -I$(ROOT)/usr/include/PR -I$(INCDIR) -I$(INCDIR)/ff -I$(ASSDIR) -I../libcart/include
LCOPTS =	-G 0
LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(NUSYSLIBDIR) -lnusys_d -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc -L../libcart/lib -lcart

OPTIMIZER =	-g

APP = $(OBJDIR)/$(TARGET).out
ROM = $(OBJDIR)/$(TARGET).n64

CODEFILES   = 	$(wildcard $(SRCDIR)\*.c) $(wildcard $(SRCDIR)\*\*.c) 

CODEOBJECTS =	$(subst $(SRCDIR),$(OBJDIR),$(CODEFILES:.c=.o))  $(NUSYSLIBDIR)/nusys.o

CODESEGMENT =	$(OBJDIR)\codesegment.o

default: $(ROM)

.PHONY: $(SRCDIR)\ff

$(SRCDIR)\ff:
  @mkdir -p $(SRCDIR)\ff

$(OBJDIR)\ff\%.o: $(SRCDIR)\ff\%.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)\%.o: $(SRCDIR)\%.c
	$(CC) $(CFLAGS) $< -o $@

$(CODESEGMENT):	$(CODEOBJECTS) Makefile
		$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(ROM):	$(CODESEGMENT)
		$(MAKEROM) spec -I$(NUSYSINCDIR) -r $(ROM) -e $(APP)
		$(MAKEMASK) $(ROM)
