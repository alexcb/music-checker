CC=gcc
CCFLAGS=-std=gnu11 -Wall -Werror -I./src -DDEBUG_BUILD=1
LDFLAGS=-lmpg123 -lpthread -lm

SRC=$(wildcard src/**/*.c src/*.c)
OBJ=$(SRC:%.c=%.o)

TESTSRC=$(wildcard tests/**/*.c tests/*.c)
TESTOBJ=$(TESTSRC:%.c=%.o)

OBJWITHOUTMAIN := $(filter-out src/main.o src/check.o,$(OBJ))

build: music-check test

music-check: $(OBJWITHOUTMAIN) src/main.o
	$(CC) $(CCFLAGS) -o music-check $^ $(LDFLAGS)

test: $(OBJWITHOUTMAIN) $(TESTOBJ)
	$(CC) $(CCFLAGS) -o test $^ $(LDFLAGS)

.PHONY: reformat
reformat:
	find -regex '.*/.*\.\(c\|h\)$$' -exec clang-format -i {} \;

# To obtain object files
%.o: %.c
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f music-check test $(OBJ) $(TESTOBJ)
