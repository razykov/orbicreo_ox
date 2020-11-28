PROJECT = bin/orbicreo

CC   = gcc
OPT  = -O0 -g -std=c99

LIBS  = -ljson-c
LIBS += -lssl
LIBS += -lcrypto
LIBS += -lpthread

OPTIMIZATION  = -O0 -g

#  Includes
INCLUDE  = -I src/

#  Compiler Options
GCFLAGS  = -Wall -Werror -Wextra $(OPTIMIZATION) $(INCLUDE) $(LIBS)

SRC  = src/main.c
SRC += src/orb_args.c
SRC += src/orb_init.c
SRC += src/orb_log.c
SRC += src/orb_utils.c
SRC += src/orb_types.c
SRC += src/orb_json.c
SRC += src/orb_utils_str.c
SRC += src/orb_threads.c
SRC += src/orb_build.c
SRC += src/orb_build_link.c
SRC += src/orb_build_compile.c

all: bin
	$(CC) $(OPT) $(SRC) $(GCFLAGS) -o $(PROJECT)

bin:
	mkdir -p bin

clean:
	rm -rf bin/*
