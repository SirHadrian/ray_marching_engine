CC=gcc
LD=gcc
CFLAGS=-Wall -Wextra -Wconversion -Wuninitialized# -Werror
LDFLAGS=-lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl
TARGET=window.out
OBJS=main.o glad.o

all: ${TARGET}
	./${TARGET}

${TARGET}: ${OBJS}
	${LD} ${OBJS} ${LDFLAGS} -o ${TARGET}

%.o: %.c
	${CC} ${CFLAGS} -c $<

clean:
	rm *.o ${TARGET}
