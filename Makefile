BINDIR = bin
SOURCEDIR = src
SHAREFS = share
PROJECT = $(BINDIR)/orbicreo
ROOTFS = rootfs/
BINDIST = $(ROOTFS)/usr/bin

VERSION_FILE=version

MAJOR=$(shell cat $(VERSION_FILE) | awk -F. '{print $$1}')
MINOR=$(shell cat $(VERSION_FILE) | awk -F. '{print $$2}')
BUILD=$(shell cat $(VERSION_FILE) | awk -F. '{print $$3}')

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
INCLUDE  = -I $(SOURCEDIR)

#  Compiler options
DEFINES = -DVERSION_MAJOR=\"$(MAJOR)\" -DVERSION_MINOR=\"$(MINOR)\" -DVERSION_BUILD=\"$(BUILD)\"
GCFLAGS = -Wall -Werror -Wextra $(DEFINES) $(STD) $(INCLUDE) $(LIBS)

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
