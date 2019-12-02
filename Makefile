target  ?= sgicdserv
objects := $(patsubst %.c,%.o,$(wildcard *.c))

libs:= libulfius libyder sqlite3

EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Wimplicit-fallthrough

ifdef libs
LDLIBS  += $(shell pkg-config --libs   ${libs})
CFLAGS  += $(shell pkg-config --cflags ${libs})
endif

LDFLAGS += ${EXTRAS}
CFLAGS  += -std=gnu17 -Og -ggdb ${EXTRAS}

.PHONY: all
all:	$(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)

$(target): $(objects)
