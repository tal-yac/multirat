OBJECTS += server.o

server: ${OBJECTS}
	gcc ${CFLAGS} ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB}

main.o: main.c net_util.h server.h client.h
	gcc ${CCFLAGS} $< -o $@

server.o: server.c server.h log.h commands.h
	gcc ${CCFLAGS} $< -o $@