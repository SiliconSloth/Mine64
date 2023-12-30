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

LCDEFS =	-DNU_DEBUG -DF3DEX_GBI_2
LCINCS =	-I. -I$(NUSYSINCDIR) -I$(ROOT)/usr/include/PR -I$(INCDIR) -I$(ASSDIR)
LCOPTS =	-G 0
LDFLAGS = $(MKDEPOPT) -L$(LIB) -L$(NUSYSLIBDIR) -lnusys_d -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc

OPTIMIZER =	-g

APP = $(OBJDIR)/$(TARGET).out
ROM = $(OBJDIR)/$(TARGET).n64

CODEFILES   = 	$(wildcard $(SRCDIR)\*.c) 

CODEOBJECTS =	$(subst $(SRCDIR),$(OBJDIR),$(CODEFILES:.c=.o))  $(NUSYSLIBDIR)/nusys.o

CODESEGMENT =	$(OBJDIR)\codesegment.o

default: $(ROM)

$(OBJDIR)\%.o: $(SRCDIR)\%.c
	$(CC) $(CFLAGS) $< -o $@

$(CODESEGMENT):	$(CODEOBJECTS) Makefile
		$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

$(ROM):	$(CODESEGMENT)
		$(MAKEROM) spec -I$(NUSYSINCDIR) -r $(ROM) -e $(APP)
