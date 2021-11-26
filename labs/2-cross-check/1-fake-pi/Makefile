TARGET = libpi-fake.a
MAKEFLAGS += --no-print-directory

# if you want to compile everything in a directory, do something like this.
# SOURCES := $(wildcard ./*.c)

# define where gpio.c is, and the programs you want to run.
LPP = $(CS140E_2021_PATH)
GPIO         := $(LPP)/labs/1-gpio/code/gpio.c

SRC :=  fake-put-get.c fake-random.c pi-random.c  fake-driver.c tape.c
OBJS := $(patsubst %.c, %.o, $(SRC)) gpio.o

ifndef CS140E_2021_PATH
$(error CS140E_2021_PATH is not set: this should contain the absolute path to where this directory is.  Define it in your shell's initialiation.  For example, .tcshrc for tcsh or .bashrc for bash)
endif


# where the libpi includes are
LPI = $(LPP)/libpi/include
# where libunix is (currently)
LUNIX = $(LPP)/libunix

CC=gcc
CFLAGS = -Wall -g -Og -std=gnu99  -Werror -Wno-unused-function  -Wno-unused-variable

CFLAGS += -I$(LPI) -I$(LUNIX) -DRPI_UNIX

# overdepend: all .o's depend on all .h's  --- make deps as simple as possible.
DEPS := $(wildcard ./*.h) $(LPI)/rpi.h Makefile

all: $(TARGET)

$(OBJS): $(DEPS) 

gpio.o: $(GPIO)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(DEPS) 
	ar cr $(TARGET) $(OBJS) 


run:
	make -C tests  runall
check:
	make -C tests  checkall
emit:
	make -C tests  emitall

clean:
	rm -f *.o $(TARGET) *~  *.bak  *.out $(OBJS)
	@make -s -C tests clean

.PHONY: all clean run check emit
