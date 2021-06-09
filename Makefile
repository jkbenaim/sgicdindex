target  ?= sgicdindex
objects := $(patsubst %.c,%.o,$(wildcard *.c))

libs:= sqlite3

#EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Wimplicit-fallthrough -flto -fanalyzer -Og -ggdb

ifdef libs
LDLIBS  += $(shell pkg-config --libs   ${libs})
CFLAGS  += $(shell pkg-config --cflags ${libs})
endif

LDFLAGS += ${EXTRAS}
CFLAGS  += -std=c99 ${EXTRAS}

.PHONY: all
all:	index.html

.PHONY: clean
clean:
	rm -f $(target) $(objects) index.html

$(target): $(objects)

index.html: $(target) sgicds.db
	./$(target) sgicds.db | xmllint --valid --output index.html -

