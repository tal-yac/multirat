CFLAGS := -g -std=gnu11
WARNNINGS := -Wall -Wextra -Werror
CCFLAGS := -c ${CFLAGS} ${WARNINGS}
WIN_TCPIP_LIB := Ws2_32

ifndef LOG_LEVEL
	LOG_LEVEL := 0
endif

CCFLAGS += -DLOG_LEVEL=${LOG_LEVEL}

OBJECTS := net_util.o ratpacket.o commands.o

SERVER_OBJECTS = server.o clients_manager.o ${OBJECTS}

server: ${SERVER_OBJECTS}
	gcc ${CFLAGS} ${SERVER_OBJECTS} -o $@ -l${WIN_TCPIP_LIB} -lpthread

client: client.o ${OBJECTS}
	gcc ${CFLAGS} $< ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB} -lpthread

net_util.o: net_util.c net_util.h
	gcc ${CCFLAGS} $< -o $@

client.o: client.c log.h commands.h
	gcc ${CCFLAGS} $< -o $@

server.o: server.c log.h commands.h clients_manager.h
	gcc ${CCFLAGS} $< -o $@

ratpacket.o: ratpacket.c ratpacket.h
	gcc ${CCFLAGS} $< -o $@

commands.o: commands.c commands.h
	gcc ${CCFLAGS} $< -o $@

clients_manager.o: clients_manager.c clients_manager.h net_util.h log.h
	gcc ${CCFLAGS} $< -o $@

.PHONEY: clean
clean:
	del /Q /F *.o *.exe