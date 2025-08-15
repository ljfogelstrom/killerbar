components = hello_world.o datetime.o status.o
required = util.h

all: statusexe

statusexe: $(components)
	cc -g -o statusexe $(components) -lX11

$(components): hello_world.c datetime.c status.c util.h helpers.h
	cc -g -c hello_world.c datetime.c status.c
clean:
	rm *.o
