OBJS = julia.cpp

CC = g++

COMPILER_FLAGS = -O2

LINKER_FLAGS = -lSDL2

OBJ_NAME = julia

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
