TARGET = Mp4Test
OBJS = main.o mem64.o

INCDIR =
CFLAGS = -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
BUILD_PRX = 1
LIBDIR =
LDFLAGS =
LIBS = -lpsppower -lm -lpsprtc -lpspgu -lmp4ff -lpspmpeg -lpspmpegbase

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = MP4 Test
PSP_LARGE_MEM=1
PSP_FW_VERSION=500

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak