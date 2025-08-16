CFLAGS = -lX11
RUNNING != pidof killbar

components = 	hello_world.o \
			 	datetime.o \
			 	run_command.o \
			 	status.o
headers = 		util.h helpers.h


debug asm all: killbar

killbar: ${components}
	cc ${CFLAGS} -o killbar ${components}

${components}: ${components:.o=.c} ${headers}
	cc ${CFLAGS} -c ${components:.o=.c}

clean: ${components}
	rm *.o *.s

install: all
	cp -f killbar /usr/local/bin/
ifdef RUNNING
	kill -9 ${RUNNING}
endif

debug: CFLAGS += -g
asm: CFLAGS += -S
