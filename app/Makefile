TARGET = NTPSP

SOURCES   := data drivers source
CFILES    := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
SFILES    := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.S))
CPPFILES  := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
FONTFILES := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.pgf))
GFXFILES  := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.png))
PRXFILES  := $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.prx))

OBJS := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(SFILES:.S=.o) $(CPPFILES:.cpp=.o) \
        $(FONTFILES:.pgf=.o) $(GFXFILES:.png=.o) $(PRXFILES:.prx=.o)

VERSION_MAJOR :=  1
VERSION_MINOR :=  0

INCDIR   = ../libs/ ../libs/include include
CFLAGS   = -Os -G0 -Wall -ffast-math -Wno-narrowing -Wno-unused-variable \
           -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -std=gnu++17
ASFLAGS  := $(CFLAGS)

BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

LIBDIR  = ../libs/lib
LDFLAGS = -nostdlib -nodefaultlibs
LIBS    = -lpspmodinfo -lpsprtc -lvlfgui -lvlfgu -lvlfutils -lvlflibc

EXTRA_TARGETS   = EBOOT.PBP
PSP_EBOOT_TITLE = NTPSP v$(VERSION_MAJOR).$(VERSION_MINOR)$(VERSION_MICRO)
# PSP_EBOOT_ICON  = ../ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include ./build.mak

%.o: %.pgf
	bin2o -i $< $@ $(addsuffix _pgf, $(basename $(notdir $<) ))

%.o: %.png
	bin2o -i $< $@ $(addsuffix _png, $(basename $(notdir $<) ))

%.o: %.prx
	bin2o -i $< $@ $(addsuffix _prx, $(basename $(notdir $<) ))
