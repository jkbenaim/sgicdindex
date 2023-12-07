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
all:	index.html index-with-ids.html DIGESTS.txt sql.txt

.PHONY: clean
clean:
	rm -f $(target) $(objects) index.html index-with-ids.html sql.txt

$(target): $(objects)

index.html: $(target) sgi.db
	./$(target) -f sgi.db | xmllint --valid --output $@ -

index-with-ids.html: $(target) sgi.db
	./$(target) -f sgi.db -i -h | xmllint --valid --output $@ -

DIGESTS.txt: $(target) sgi.db mkdigests.sql
	sqlite3 sgi.db < mkdigests.sql > $@

sql.txt: sgi.db
	sqlite3 sgi.db ".dump --preserve-rowids" > sql.txt
