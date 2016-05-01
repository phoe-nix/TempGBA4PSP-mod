TARGET = ku_bridge
OBJS = main.o sceCtrl_driver.o

INCDIR =
CFLAGS = -O2 -G0 -Wall -march=allegrex -mfp32 -mgp32 -mlong32 -mabi=eabi
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = $(TARGET).exp

PSP_FW_VERSION=150

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR =
LDFLAGS = -mno-crt0 -nostartfiles
LIBS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	psp-build-exports -s $(PRX_EXPORTS)
	@cp $(TARGET).prx ../
	@cp $(TARGET).S ../
	@cp $(TARGET).h ../
	@rm -f $(TARGET).prx
	@rm -f $(TARGET).S
	@rm -f *.o
	@rm -f *.elf

