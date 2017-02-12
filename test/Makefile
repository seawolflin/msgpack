
SRC :=  ../msgpack.c \
		  test_main.c \
		  msgpack_unittest.c

OBJS := $(SRC:.c=.o)

CC = gcc

INCLUDE = -I. -I../include -Icommom

CFLAGS := -g -Os

all: msgpack_test
	@echo "Enter regular: all..."


msgpack_test: $(OBJS)
	gcc -o $@ $^


%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

.PHONY:all clean print

rwildcard=$(strip $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d)))

clean:
	-rm -rf $(call rwildcard,,*.o) msgpack_test

print:
	@echo $(OBJS)

