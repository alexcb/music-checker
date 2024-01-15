CC=gcc
CCFLAGS=-std=gnu11 -Wall -Werror -I./src -DDEBUG_BUILD=1
LDFLAGS=-lmpg123 -lpthread -lm

DECODESRC=$(wildcard src/mp3decode/*.c)
DECODEOBJ=$(DECODESRC:%.c=%.o)

WALKSRC=$(wildcard src/mp3walk/*.c)
WALKOBJ=$(WALKSRC:%.c=%.o)

COMMONSRC=$(wildcard src/common/*.c)
COMMONOBJ=$(COMMONSRC:%.c=%.o)

build: mp3decode mp3walk

mp3decode: $(DECODEOBJ) $(COMMONOBJ) src/mp3decode/main.o
	$(CC) $(CCFLAGS) -o mp3decode $^ $(LDFLAGS)

mp3walk: $(WALKOBJ) $(COMMONOBJ) src/mp3walk/main.o
	$(CC) $(CCFLAGS) -o mp3walk $^ $(LDFLAGS)

.PHONY: reformat
reformat:
	find -regex '.*/.*\.\(c\|h\)$$' -exec clang-format -i {} \;

# To obtain object files
%.o: %.c
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f music-check $(DECODEOBJ) $(WALKOBJ) $(COMMONOBJ)
