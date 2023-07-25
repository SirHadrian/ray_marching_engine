COMPILER=gcc
COMPILER_FLAGS=-Wall -Wextra -Wconversion -Wuninitialized -Werror
LIBS=-lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl

default: main.o glad.o
	${COMPILER} main.o glad.o -o window.out ${COMPILER_FLAGS} ${LIBS} && ./window.out

main.o: main.c
	${COMPILER} -c main.c

glad.o: glad.c
	${COMPILER} -c glad.c

build: main.o glad.o
	${COMPILER} main.o glad.o -o window.out ${COMPILER_FLAGS} ${LIBS}
