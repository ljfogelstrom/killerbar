CFLAGS = -g -lX11 -pthread -Wall
RUNNING != pidof killerbar

objects = 	hello_world.o \
			 	datetime.o \
			 	run_command.o \
			 	main.o
headers = 		util.h helpers.h
bin = /usr/local/bin


all: killerbar

killerbar: ${objects}
	cc ${CFLAGS} -o killerbar ${objects}

${objects}: ${objects:.o=.c} ${headers}
	cc ${CFLAGS} -c ${objects:.o=.c}

clean: ${objects}
	rm *.o *.s

install: all
	cp -f killerbar ${bin}
ifdef RUNNING
	kill -9 ${RUNNING}
	./killerbar &
endif

uninstall:
	rm ${bin}/killerbar

debug: all
	gdb --args killerbar -v

asm: CFLAGS += -S
