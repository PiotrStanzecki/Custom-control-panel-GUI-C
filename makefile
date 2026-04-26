CC = gcc

RAYLIB_DIR = ./raylib


CFLAGS = -Wall -std=c99 

INC = -I$(RAYLIB_DIR)/include
LDFLAGS = -L$(RAYLIB_DIR)/lib

LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread

.PHONY: all clean

all: clean main.exe run

main.exe: main.c
	$(CC) main.c -o main.exe $(INC) $(LDFLAGS) $(CFLAGS) $(LIBS)
	
run: main.exe
	./main.exe

clean:
	del main.exe