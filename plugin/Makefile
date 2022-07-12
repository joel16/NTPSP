TARGET = rtc_driver
OBJS = rtc_driver.o exports.o

PRX_EXPORTS = exports.exp

# Use the kernel's small inbuilt libc
USE_KERNEL_LIBC = 1
# Use only kernel libraries
USE_KERNEL_LIBS = 1

INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LDFLAGS = -nostartfiles
LIBS = -lpsprtc_driver

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	psp-build-exports -s $(PRX_EXPORTS)
	mkdir "../app/data/"; mv rtc_driver.prx "../app/data/"
	mkdir "../app/drivers/"; mv rtc_driver.S "../app/drivers/"
