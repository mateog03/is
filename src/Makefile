CC= gcc

SRC= $(wildcard *.c)

WARN= -Wall -Wextra -pedantic
DEBG= -g -O0 -fno-omit-frame-pointer -fsanitize=address
OPT=  -O2 -flto -DNDEBUG
BIN=  is

all:
	$(CC) $(WARN) $(DEBG) $(SRC) -o $(BIN)

build:
	$(CC) $(WARN) $(OPT) $(SRC) -o $(BIN)

clean:
	@rm -f $(BIN)
