CC = gcc
FILES = toralizer.c
FLAGS = -g

all:
	$(CC) $(FLAGS) -o ./toralizer $(FILES)

run:
	./toralizer $(ARGS)

clean:
	rm -rf ./toralizer
