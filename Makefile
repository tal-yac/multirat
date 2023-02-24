CFLAGS := -g -std=gnu11
WARNNINGS := -Wall -Wextra -Werror
CCFLAGS := -c ${CFLAGS} ${WARNINGS}
WIN_TCPIP_LIB := Ws2_32

LOG_LEVEL ?= 0

OBJECTS := net_util.o ratpacket.o commands.o

ifeq "$(MAKECMDGOALS)" "server"
CCFLAGS += -DSERVER_SIDE
OBJECTS += clients_manager.o server.o
else
CCFLAGS += -DCLIENT_SIDE
OBJECTS += client.o
endif

CCFLAGS += -DLOG_LEVEL=${LOG_LEVEL}

server: clean ${OBJECTS}
	gcc ${CFLAGS} ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB} -lpthread

client: clean ${OBJECTS}
	gcc ${CFLAGS} ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB} -lpthread

net_util.o: net_util.c net_util.h
	gcc ${CCFLAGS} $< -o $@

client.o: client.c log.h commands.h
	gcc ${CCFLAGS} $< -o $@

server.o: server.c log.h commands.h clients_manager.h
	gcc ${CCFLAGS} $< -o $@

ratpacket.o: ratpacket.c ratpacket.h
	gcc ${CCFLAGS} $< -o $@

commands.o: commands.c commands.h log.h
	gcc ${CCFLAGS} $< -o $@

clients_manager.o: clients_manager.c clients_manager.h net_util.h log.h
	gcc ${CCFLAGS} $< -o $@

.PHONEY: clean
clean:
	del /Q /F *.o *.exe