CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
SOURCES = main.c model.c input.c matching.c
TARGET = lost_found

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES) model.h input.h matching.h
	$(CC) $(CFLAGS) $(SOURCES) -o $@

test:
	sh tests/test_workflows.sh

clean:
	rm -f $(TARGET)
