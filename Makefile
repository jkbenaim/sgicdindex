target  ?= sgicdserv
objects := $(patsubst %.c,%.o,$(wildcard *.c))

libs := libulfius libyder sqlite3

EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check

LDFLAGS += $(shell pkg-config --libs ${libs}) ${EXTRAS}
CFLAGS += $(shell pkg-config --cflags ${libs}) -Og -ggdb ${EXTRAS}

.PHONY: all
all:	$(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)

$(target): $(objects)
