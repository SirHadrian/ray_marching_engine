CC=gcc
CFLAGS=-Wall -Wextra -Wconversion -Wuninitialized # -Werror
LIBS=-lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl
TARGET=window.out

default: main.o glad.o
	${CC} main.o glad.o -o window.out ${LIBS} && ./${TARGET}

main.o: main.c
	${CC} ${CFLAGS} -c main.c

glad.o: glad.c
	${CC} ${CFLAGS} -c glad.c

clean:
	rm *.o ${TARGET}
