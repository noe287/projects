
DIRS =  lib \
	client-server

all:
	@ echo ${DIRS}
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE}); \
		if test $$? -ne 0; then break; fi; done

clean:
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} clean); done
