CC=gcc -std=c99
CFLAGS=-c -Wall -Wextra
LDFLAGS=-g
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=clue

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	rm *o

.c.o:
	$(CC) $(CFLAGS) $< -o $@
