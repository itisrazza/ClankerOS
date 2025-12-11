# Hello World Program Makefile

# Include common configuration
include ../../Makefile.common

TARGET = hello
OBJS = main.o

CFLAGS = -std=c11 -O2 -g -Wall -Wextra -Wpedantic
LDFLAGS =

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
