# Compiler and flags
CC = gcc
CFLAGS = -lm -lpthread 

# Source files
SRC = hw1_21050111058.c
OUTPUT = hw1_21050111058

all: $(OUTPUT)
	@echo "Usage: make run IMAGE=<image file> THREADS=<number of thread>"
$(OUTPUT): $(SRC) 
	$(CC) $(SRC) $(CFLAGS) -o $(OUTPUT)


run: $(OUTPUT)
	
	./$(OUTPUT) $(IMAGE) $(THREADS)

# Clean up the output file
clean:
	rm -f $(OUTPUT)

