
SRC :=  ../msgpack.c \
		  msgpack_person.c

OBJS := $(SRC:.c=.o)

CC = gcc

INCLUDE = -I. -I../include -Icommom

CFLAGS := -g -Os

all: msgpack_person
	@echo "Enter regular: all..."


msgpack_person: $(OBJS)
	gcc -o $@ $^


%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

.PHONY:all clean print

rwildcard=$(strip $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d)))

clean:
	-rm -rf $(call rwildcard,,*.o) msgpack_person

print:
	@echo $(OBJS)

