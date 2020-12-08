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
SRC += src/orb_utils/orb_args.c
SRC += src/orb_init/orb_goal_init.c
SRC += src/orb_utils/orb_log.c
SRC += src/orb_utils/orb_utils.c
SRC += src/orb_types/orb_types.c
SRC += src/orb_types/orb_json.c
SRC += src/orb_utils/orb_utils_str.c
SRC += src/orb_utils/orb_threads.c
SRC += src/orb_build/orb_build_utils.c
SRC += src/orb_build/orb_build_link.c
SRC += src/orb_build/orb_build_compile.c
SRC += src/orb_build/orb_build_mkinclude.c
SRC += src/orb_build/orb_goal_build.c
SRC += src/orb_list/orb_goal_list.c

all: bin
	$(CC) $(OPT) $(SRC) $(GCFLAGS) -o $(PROJECT)

bin:
	mkdir -p bin

clean:
	rm -rf bin/*
