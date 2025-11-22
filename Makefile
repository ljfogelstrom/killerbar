CC = gcc
CFLAGS = -Wall -Wextra -O2 -fcommon
LDFLAGS = -lX11

SRC_DIR = ./src
BUILD_DIR = ./build

TARGET = killerbar

OBJS = $(addprefix ${BUILD_DIR}/, datetime.o \
		run_command.o \
		ram.o \
		cpu.o \
		util.o \
		disk.o \
		cat.o \
		temperature.o \
		main.o)

all: always ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c $< -o $@ ${LDFLAGS}

always:
	[[ -d ${BUILD_DIR} ]] || mkdir -p ${BUILD_DIR}

clean:
	rm -rf ${BUILD_DIR} ${TARGET}
