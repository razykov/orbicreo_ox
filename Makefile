BINDIR = bin
SOURCEDIR = src
SHAREFS = share
PROJECT = $(BINDIR)/orbicreo
ROOTFS = rootfs/
BINDIST = $(ROOTFS)/usr/bin

CC    = gcc
STRIP = strip

LIBS  = -ljson-c
LIBS += -lssl
LIBS += -lcrypto
LIBS += -lpthread

DEBUG_OPT = -O0 -g
RELEASE_OPT = -O2
STD = -std=c99

#  Includes
INCLUDE  = -I src/

#  Compiler Options
GCFLAGS  = -Wall -Werror -Wextra $(STD) $(INCLUDE) $(LIBS)

SRC = $(shell find $(SOURCEDIR) -name '*.c')

debug: bin
	$(CC) $(DEBUG_OPT) $(GCFLAGS) $(SRC) -o $(PROJECT)

release: bin
	$(CC) $(RELEASE_OPT) $(GCFLAGS) $(SRC) -o $(PROJECT)
	$(STRIP) $(PROJECT)

all: debug

bin:
	@mkdir --parents $(BINDIR)

clean:
	@rm --recursive --force $(BINDIR)

install:
	@mkdir --parents $(BINDIST)
	@cp $(PROJECT) $(BINDIST)
	@cp --recursive $(SHAREFS)/* $(ROOTFS)
