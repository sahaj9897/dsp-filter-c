# Makefile for dsp-filter-c
# Usage:
#   make        -> build the C program
#   make run    -> generate signal, run C program, compare with Python
#   make clean  -> delete build and output files

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
SRC     = src/main.c src/filters.c
TARGET  = filter_app

all: $(TARGET)

$(TARGET): $(SRC) src/filters.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	python3 python/generate_signal.py
	./$(TARGET)
	python3 python/compare.py

clean:
	rm -f $(TARGET) $(TARGET).exe output/*.txt
