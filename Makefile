CFLAGS := -g -std=gnu11
WARNNINGS := -Wall -Wextra -Werror
CCFLAGS := -c ${CFLAGS} ${WARNINGS}
WIN_TCPIP_LIB := Ws2_32

ifndef LOG_LEVEL
	LOG_LEVEL := 0
endif

CCFLAGS += -DLOG_LEVEL=${LOG_LEVEL}

OBJECTS := net_util.o ratpacket.o commands.o

server: server.o ${OBJECTS}
	gcc ${CFLAGS} $< ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB}

client: client.o ${OBJECTS}
	gcc ${CFLAGS} $< ${OBJECTS} -o $@ -l${WIN_TCPIP_LIB}

net_util.o: net_util.c net_util.h
	gcc ${CCFLAGS} $< -o $@

client.o: client.c client.h log.h commands.h
	gcc ${CCFLAGS} $< -o $@

server.o: server.c server.h log.h commands.h
	gcc ${CCFLAGS} $< -o $@

ratpacket.o: ratpacket.c ratpacket.h
	gcc ${CCFLAGS} $< -o $@

commands.o: commands.c commands.h
	gcc ${CCFLAGS} $< -o $@

.PHONEY: clean
clean:
	del /Q /F *.o *.exe