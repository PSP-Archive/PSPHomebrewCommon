TARGET_LIB = c:\psp\pspsdk\psp\sdk\lib\libpsphbc.a

SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCES)) callbacks.o graphics.o mem64.o

INCDIR = 
CFLAGS = -O0 -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
#LIBS = -lstdc++ -lpspgu -lpspgum -lpng -lz -lm

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak