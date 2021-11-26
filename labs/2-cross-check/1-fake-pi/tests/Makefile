# trivial makefile: can define the SRC here or pass it in.
# note: this is useful or dangerous: we strip out all the 
# directory stuff so we can put the executable here.

MAKEFLAGS += --no-print-directory

# define the tests you want to compile here.  this would 
# be the whole set:
#TEST_SRC := $(wildcard ./[01]-*.c)                      \
#            $(wildcard ./[34]-unix-*.c)                 \
# $(CS140E_2021_PATH)/labs/1-gpio/code.staff/1-blink.c   \
# $(CS140E_2021_PATH)/labs/1-gpio/code.staff/2-blink.c 

# do the first easy set
# TEST_SRC := $(wildcard ./[01]-*.c) $(wildcard ./[34]-unix-*.c)
# TEST_SRC := $(wildcard ./[015]-*.c) $(wildcard ./[34]-unix-*.c) $(CS140E_2021_PATH)/labs/1-gpio/code/1-blink.c
TEST_SRC := $(wildcard ./[5]-*.c)

# make them local to this directory
TESTS := $(patsubst %.c, %, $(TEST_SRC))
# remove path from names
TEST_PROGS := $(foreach src, ./$(TESTS), $(notdir $(src))) 

# pull out all the directories in the test files so make knows 
# where to look for everything.
# note: if tests have the same name, you'll get a collision.
VPATH := $(sort $(dir $(TESTS)))

LPP = $(CS140E_2021_PATH)/libpi
LIBPI_FAKE = ../libpi-fake.a
LPI = -I$(LPP)/include

CC = gcc
CFLAGS = -DRPI_UNIX $(LPI) -I$(LPP) -I../ -Wall -Wno-unused-function -Wno-unused-variable 

DEPS = ./Makefile $(LIBPI_FAKE) $(TEST_PROGS)

all: libs $(TEST_PROGS) 

$(LIBPI_FAKE):
	@make -s -C ../  libpi-fake.a

#$(TEST_UNIX_PROGS): ./%: %.c 
#	$(CC) $(CFLAGS) $< -o $@

# we put all the executables here: how to remove?
./%: %.c  $(LIBPI_FAKE) 
	$(CC) $(CFLAGS) $< -o $(notdir $@)  $(LIBPI_FAKE)

# i'm not sure this actually makes things better.
emitall: $(DEPS)
	@make -f ./Makefile.test emitall "TESTS=$(TEST_PROGS)"
runall: $(DEPS)
	@make -f ./Makefile.test runall "TESTS=$(TEST_PROGS)"
checkall: $(DEPS)
	@make -f ./Makefile.test checkall "TESTS=$(TEST_PROGS)"
cksumall: $(DEPS)
	@make -f ./Makefile.test cksumall "TESTS=$(TEST_PROGS)"

clean:
	rm -rf $(TEST_PROGS) *~ tags *.o Makefile.bak

cleanall:
	@make -s -f ./Makefile clean
	@make -s -f ./Makefile.test cleanall TESTS=x

.PHONY: all clean libs runall checkall emitall
