# Makefile for myShell
CC = gcc                                      # C compiler to use
CFLAGS = -Wall -Wextra -std=c99              # Compiler flags (warnings + C99 standard)
TARGET = myKonsole                              # Name of executable to produce
SRCS = myKonsole.c                              # Source file to compile

all: $(TARGET)                                # Default target builds executable

$(TARGET): $(SRCS)                            # Link source to create executable
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)      # Compile command

clean:                                        # Remove compiled files
	rm -f $(TARGET)                           # Delete executable

run: $(TARGET)                                # Build and run the shell
	./$(TARGET)                               # Execute the program

.PHONY: all clean run                         # Phony targets (not actual files)