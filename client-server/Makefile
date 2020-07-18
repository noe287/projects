include ../Makefile.inc

GEN_EXE = routingmanager \
	  client \

EXE = ${GEN_EXE} ${LINUX_EXE}

all: ${EXE}

clean :
	${RM} ${EXE} *.o

showall:
	@ echo ${EXE}

${EXE} : ${SOCKET_LIB}


routingmanager: routingmanager.o
	${CC} -o $@ routingmanager.o \
		${CFLAGS} ${IMPL_LDLIBS} ${IMPL_THREAD_FLAGS}
