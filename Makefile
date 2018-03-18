CC = clang++
CFLAGS = -Wall -Wextra -pedantic -Wconversion -g -std=c++1z
FILE = main.cpp
OUT = output.out

build:
	@$(CC) $(CFLAGS) $(FILE) -o $(OUT)

run:
	@./$(OUT)

clean:
	@rm $(OUT)