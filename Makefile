################################################################################
# Variables
################################################################################

CFLAGS=-g -O3 -Wall -Wextra -Wno-strict-overflow -Wno-self-assign -std=c99 -D_FILE_OFFSET_BITS=64
LIBS=-lluajit-5.1 -lzmq -ldl

SOURCES=$(wildcard src/**/*.c src/**/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})
BIN_SOURCES=src/skyd.c,src/sky_gen.c
BIN_OBJECTS=$(patsubst %.c,%.o,${BIN_SOURCES})
LIB_SOURCES=$(filter-out ${BIN_SOURCES},${SOURCES})
LIB_OBJECTS=$(filter-out ${BIN_OBJECTS},${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c tests/**/*_tests.c)
TEST_OBJECTS=$(patsubst %.c,%,${TEST_SOURCES})

LUAJIT_FLAGS=-pagezero_size 10000 -image_base 100000000

PREFIX?=/usr/local

.PHONY: test valgrind

################################################################################
# Main Targets
################################################################################

compile: bin/libsky.a bin/skyd bin/sky-gen
all: compile test


################################################################################
# Installation
################################################################################

install: all
	install -d $(DESTDIR)/$(PREFIX)/sky/
	install -d $(DESTDIR)/$(PREFIX)/sky/bin/
	install -d $(DESTDIR)/$(PREFIX)/sky/data/
	install bin/skyd $(DESTDIR)/$(PREFIX)/sky/bin/
	rm $(DESTDIR)/$(PREFIX)/bin/skyd
	ln -s $(DESTDIR)/$(PREFIX)/sky/bin/skyd $(DESTDIR)/$(PREFIX)/bin/skyd

################################################################################
# Binaries
################################################################################

bin/libsky.a: bin ${LIB_OBJECTS}
	rm -f bin/libsky.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

bin/skyd: bin ${OBJECTS} bin/libsky.a
	$(CC) $(CFLAGS) -Isrc $(LUAJIT_FLAGS) -o $@ src/skyd.c bin/libsky.a $(LIBS)
	chmod 700 $@

bin/sky-gen: bin ${OBJECTS} bin/libsky.a
	$(CC) $(CFLAGS) $(LUAJIT_FLAGS) src/sky_gen.o -o $@ bin/libsky.a $(LIBS)
	chmod 700 $@

bin:
	mkdir -p bin


################################################################################
# jsmn
################################################################################

src/jsmn/jsmn.o: src/jsmn/jsmn.c
	$(CC) $(CFLAGS) -Wno-sign-compare -Isrc -c -o $@ $<



################################################################################
# Memory Leak Check
################################################################################

valgrind: VALGRIND=valgrind
valgrind: CFLAGS+=-DVALGRIND
valgrind: clean all


################################################################################
# Release Build (No Debug)
################################################################################

release: CFLAGS+=-DNDEBUG
release: clean all


################################################################################
# Tests
################################################################################

test: $(TEST_OBJECTS) tmp
	@sh ./tests/runtests.sh $(VALGRIND)

$(TEST_OBJECTS): %: %.c bin/libsky.a
	$(CC) $(CFLAGS) -Isrc $(LUAJIT_FLAGS) -o $@ $< $(LIBS) bin/libsky.a


################################################################################
# Misc
################################################################################

tmp:
	mkdir -p tmp

clean: 
	rm -rf bin ${OBJECTS} ${TEST_OBJECTS}
	rm -rf tests/*.dSYM tests/**/*.dSYM
	rm -rf  tests/*.o tests/**/*.o
	rm -rf tmp/*
