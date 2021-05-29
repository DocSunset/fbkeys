VERSION = 0.0.0

# paths
PREFIX = /usr/local

# flags
DEBUGFLAGS = -g -O0 -Wall -Werror -pedantic-errors -ansi ${INCS} -DVERSION=\"${VERSION}\" -DDEBUG
CFLAGS = -Os -Wall -Werror -pedantic-errors -ansi ${INCS} -DVERSION=\"${VERSION}\"

# compiler
CC = cc
