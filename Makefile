CFLAGS = -Wall -O2 -fcommon -Wextra -Wno-missing-field-initializers
LDFLAGS = -lX11

BUILD = ./build

VPATH = src:.
OBJ = $(addprefix ${BUILD}/,\
      	main.o \
	temperature.o \
	datetime.o \
	cat.o \
	run_command.o \
	util.o \
	volume.o \
	cpu.o \
	disk.o \
	ram.o)

TARGET = killerbar

all: ${BUILD} ${TARGET}

${TARGET}: ${OBJ} src/util.h src/status.h config.h
	${CC} ${CFLAGS} -o $@ ${OBJ} ${LDFLAGS}

${BUILD}/%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@ ${LDFLAGS}

${BUILD}:
	mkdir -p $@

clean:
	rm -rf ${BUILD} ${TARGET}

install:
	chmod 755 killerbar
	cp -f killerbar /usr/local/bin/killerbar

.PHONY: all clean install
