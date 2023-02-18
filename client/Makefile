
OBJECTS += client.o commands.o

client: ${OBJECTS}
	gcc ${CFLAGS} ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB}

main.o: main.c net_util.h client.h
	gcc ${CCFLAGS} $< -o $@

client.o: client.c client.h log.h commands.h
	gcc ${CCFLAGS} $< -o $@

commands.o: commands.c commands.h
	gcc ${CCFLAGS} $< -o $@