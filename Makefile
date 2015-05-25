###MAKEFILE CONFIGURATION FOR LINARIO TOOLCHAIN ####

# edit toolchain specs #

TC_ARCH	=	arm
TC_VENDOR=
TC_OS = 	linux
TC_ABI = 	gnueabihf
TC_GCCVER = 	4.8.3

TC_DIR=/home/gigi/development/toolchains/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux

# ! no need to edit beyond this line ! #

TC_TUPLE = 	$(TC_ARCH)-$(TC_OS)-$(TC_ABI)

CROSS_COMPILE = $(TC_DIR)/bin/$(TC_TUPLE)-
CLAGS_OPTIONS = -O2

RM		= rm -f
CP		= cp
AS		= $(CROSS_COMPILE)as
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar -q
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

TC_INCLUDE =	-I$(TC_DIR)/$(TC_TUPLE)/libc/usr/include/arm-linux-gnueabihf \
		-I$(TC_DIR)/$(TC_TUPLE)/include \
		-I$(TC_DIR)/$(TC_TUPLE)/libc/usr/include \
		-I$(TC_DIR)/lib/gcc/$(TC_TUPLE)/$(TC_GCCVER)/include-fix \
		-I$(TC_DIR)/lib/gcc/$(TC_TUPLE)/$(TC_GCCVER)/include \
		-I$(TC_DIR)/lib/gcc/$(TC_TUPLE)/$(TC_GCCVER)/finclude \

TC_LIBRARY = 	-L$(TC_DIR)/$(TC_TUPLE)/lib \
		-L$(TC_DIR)/libexec/gcc/$(TC_TUPLE)/$(GCCVER) \
		-L$(TC_DIR)/lib/gcc/$(TC_TUPLE)/$(GCCVER) \
		-L$(TC_DIR)/$(TC_TUPLE)/libc/lib/$(TC_TUPLE) \
		-L$(TC_DIR)/$(TC_TUPLE)/libc/lib \

#### TOOLCHAIN CONFIG END ####

# Program specific configuration #
BINDIR=./cross_bin
SRCDIR=./
LIBDIR=./
INCDIR=./

LIBSRC=$(LIBDIR)
MYLIB=$(LIBDIR)/mylib.a

INCLUDE =	-I$(SRCDIR) \
		-I$(LIBSRC) \
		-I$(INCDIR) \

LIBRARY =

SRCS =	$(SRCDIR)/main.c \
	$(SRCDIR)/wrap_time.c \


CFLAGS= $(CLAGS_OPTIONS) $(TC_INCLUDE) $(INCLUDE) $(TC_LIBRARY) $(LIBRARY) $(DEFINE)

DEPEND	= makedepend


# Build configuration #
.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $*.c

all:	 test_rt


test_rt:  $(SRCDIR)/main.o $(SRCDIR)/wrap_time.o 
	$(CC) -lrt -ldl -pthread $(CFLAGS) -o $(BINDIR)/test_rt $(SRCDIR)/main.o $(SRCDIR)/wrap_time.o
	$(STRIP) $(BINDIR)/test_rt


depend:
	$(DEPEND) $(TC_INCLUDE) $(INCLUDE) $(SRCS)

clean:
	@$(RM) $(BINDIR)/*
	@$(RM) $(SRCDIR)/*.o
	@$(RM) $(LIBSRC)/*.o
	@$(RM) $(SRCDIR)/*~
	@$(RM) $(LIBSRC)/*~
	make depend

remake:	clean_all all