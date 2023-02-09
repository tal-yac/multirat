all: main

CFLAGS := -g -std=gnu11
CCFLAGS := -c ${CFLAGS}
WIN_TCPIP_LIB := Ws2_32

main: main.o net_util.o server.o client.o
	gcc ${CFLAGS} main.o net_util.o server.o client.o -o main -l${WIN_TCPIP_LIB}

main.o: net_util.h server.h client.h
	gcc ${CCFLAGS} main.c -o main.o

net_util.o: net_util.h
	gcc ${CCFLAGS} net_util.c -o net_util.o

client.o: client.h
	gcc ${CCFLAGS} client.c -o client.o

server.o: server.h
	gcc ${CCFLAGS} server.c -o server.o

.PHONEY: clean
clean: 
	del /Q /F *.o *.exe