default: main.o glad.o
	gcc main.o glad.o -o window.out -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -Wall -Wextra -Wconversion -Wuninitialized -Werror && ./window.out

main.o: main.c
	gcc -c main.c

glad.o: glad.c
	gcc -c glad.c

build: main.o glad.o
	gcc main.o glad.o -o window.out -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -Wall -Wextra -Wconversion -Wuninitialized -Werror
