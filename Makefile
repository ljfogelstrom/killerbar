CFLAGS = -Wall -O2 -fcommon
LDFLAGS = -lX11

BUILD = ./build
SRC_DIR = ./src

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

${TARGET}: ${OBJ} src/util.h src/status.h
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

${BUILD}/%.o: ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c $< -o $@ ${LDFLAGS}

${BUILD}:
	mkdir -p $@

clean:
	rm -rf ${BUILD} ${TARGET}

.PHONY: all clean
