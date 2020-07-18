
SOCKET_DIR = ..
SOCKET_LIB = ${SOCKET_DIR}/socket.a
SOCKET_INC_DIR = ${SOCKET_DIR}/lib

NUX_LIBRT = -lrt
LINUX_LIBDL = -ldl
LINUX_LIBACL = -lacl
LINUX_LIBCRYPT = -lcrypt
LINUX_LIBCAP = -lcap


# "-Wextra" is a more descriptive synonym for "-W", but only
# available in more recent gcc versions

# Defining _DEFAULT_SOURCE is a workaround to avoid the warnings that
# would otherwise be produced when compiling code that defines _BSD_SOURCE
# or _SVID_SOURCE against glibc headers in version 2.20 and later.
# (The alternative would be to replace each instance of "#define _SVID_SOURCE"
# or "#define _BSD_SOURCE" in the example programs with
# "#define _DEFAULT_SOURCE".)

IMPL_CFLAGS = -std=c99 -D XOPEN_SOURCE=600 \
                -D_DEFAULT_SOURCE \
                -g -I${SOCKET_INC_DIR} \
                -pedantic \
                -Wall \
                -W \
                -Wmissing-prototypes \
                -Wno-sign-compare \
                #-Wimplicit-fallthrough \
                -Wno-unused-parameter

# clang(1) is a little more zealous than gcc(1) with respect to some warnings.
# Suppress those warnings (which, at least in the book code, relate to code
# that is fine).

# clang(1) is a little more zealous than gcc(1) with respect to some warnings.
# Suppress those warnings (which, at least in the book code, relate to code
# that is fine).

ifeq ($(CC), clang)
        IMPL_CFLAGS += -Wno-uninitialized -Wno-infinite-recursion \
                       -Wno-format-pedantic
endif

CFLAGS = ${IMPL_CFLAGS}

IMPL_THREAD_FLAGS = -pthread

IMPL_LDLIBS = ${SOCKET_LIB}

LDLIBS = ${IMPL_LDLIBS}

RM = rm -f



