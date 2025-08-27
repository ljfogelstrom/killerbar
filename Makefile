flags = -lX11 -pthread -Wall
running != pidof killerbar
objects = 	datetime.o \
		run_command.o \
		ram.o \
		cpu.o \
		util.o \
		disk.o \
		cat.o \
		temperature.o \
		main.o
bin = /usr/local/bin


all: killerbar

killerbar: ${objects}
	${CC} ${flags} ${CFLAGS} -o $@ $^

${objects}: ${objects:.o=.c}
	${CC} ${flags} ${CFLAGS} -c $^

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
