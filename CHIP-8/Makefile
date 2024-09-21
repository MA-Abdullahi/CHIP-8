INCLUDES = -I ./include
FLAGS = -g -fsanitize=address -std=gnu99
OBJECTS = ./build/chip8.o ./build/keyboard.o
OUTPUT = ./bin/main

all: ${OBJECTS}
	gcc ${FLAGS} ${INCLUDES} ./src/main.c ${OBJECTS} -lSDL2main -lSDL2 -o ${OUTPUT}

./build/chip8.o: ./src/chip8.c
	gcc ${FLAGS} ${INCLUDES} ./src/chip8.c -c -o ./build/chip8.o

./build/keyboard.o: ./src/keyboard.c
	gcc ${FLAGS} ${INCLUDES} ./src/keyboard.c -c -o ./build/keyboard.o

clean:
	rm -rf ./build/*
