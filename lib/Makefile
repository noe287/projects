include ../Makefile.inc

all : ${SOCKET_LIB}

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

${SOCKET_LIB} : ${OBJECTS}
	${RM} ${SOCKET_LIB}
	${AR} rs ${SOCKET_LIB} *.o

clean :
	${RM} *.o ${SOCKET_LIB}
