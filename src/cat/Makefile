CC=gcc
FLAGS=-Wall -Werror -Wextra -pedantic
TARGET=s21_cat
SRC=
UNAME := $(shell uname -s)
SRC := s21_cat.c

OBJ=$(SRC.c=.o)

all: $(TARGET)

%.o: %.c
	$(CC) -c $(FLAGS) $<  -o $@

$(TARGET): $(OBJ)
	$(CC) $(FLAGS) $(SRC) -o $@

clean:
	rm -rf $(OBJ) $(TARGET) *.out *.log *.dSYM

rebuild: clean all