# Compiler
CC = gcc

# Project files
SRC = src/main.c
OBJ = $(SRC:.c=.o)
EXEC = main

# Raylib paths
RAYLIB_INC = /opt/homebrew/include
RAYLIB_LIB = /opt/homebrew/lib

# Compilation flags
CFLAGS = -I$(RAYLIB_INC) -Wall -std=c11
LDFLAGS = -L$(RAYLIB_LIB) -lraylib -lm -lpthread -ldl \
          -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

# Default target
all: $(EXEC)

# Compile program
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(EXEC)

# Run program
run: $(EXEC)
	./$(EXEC)

