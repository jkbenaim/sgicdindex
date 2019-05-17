target  ?= sgicdserv
objects := $(patsubst %.c,%.o,$(wildcard *.c))

libs := libulfius libyder sqlite3

LDFLAGS += $(shell pkg-config --libs ${libs})
CFLAGS += $(shell pkg-config --cflags ${libs}) -Og -ggdb

.PHONY: all
all:	$(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)

$(target): $(objects)
