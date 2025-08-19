flags = -lX11 -pthread -Wall
running != pidof killerbar
objects = 	hello_world.o \
			 	datetime.o \
			 	run_command.o \
				cpu.o \
				util.o \
				disk.o \
				cat.o \
			 	main.o
bin = /usr/local/bin


all: killerbar

killerbar: ${objects}
	cc ${flags} ${CFLAGS} -o killerbar $^

${objects}: ${objects:.o=.c}
	cc ${flags} ${CFLAGS} -c $^

clean: ${objects}
	rm *.o 
	rm *.s

install: all
	cp -f killerbar ${bin}
ifdef running
	kill -9 ${running}
endif

uninstall:
	rm ${bin}/killerbar

debug: all
	gdb --args killerbar -v

CFLAGS =
