objects := db.o errsql.o escape.o

libs:= sqlite3

#EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Wimplicit-fallthrough -flto -fanalyzer -Og -ggdb

ifdef libs
LDLIBS  += $(shell pkg-config --libs   ${libs})
CFLAGS  += $(shell pkg-config --cflags ${libs})
endif

LDFLAGS += ${EXTRAS}
CFLAGS  += -std=c99 ${EXTRAS}

.PHONY: all
all:	index.html index-with-ids.html DIGESTS.txt sql.txt hw.html

.PHONY: clean
clean:
	rm -f mkcds mkhw mkcds.o mkhw.o $(objects) index.html index-with-ids.html sql.txt

mkcds: mkcds.o $(objects)

mkhw: mkhw.o $(objects)

index.html: mkcds sgi.db
	./mkcds -f sgi.db | xmllint --valid --output $@ -

index-with-ids.html: mkcds sgi.db
	./mkcds -f sgi.db -i -h | xmllint --valid --output $@ -

DIGESTS.txt: mkcds sgi.db mkdigests.sql
	sqlite3 sgi.db < mkdigests.sql > $@

sql.txt: sgi.db
	sqlite3 sgi.db ".dump --preserve-rowids" > sql.txt

hw.html: mkhw sgi.db
	./mkhw -f sgi.db | xmllint --valid --output $@ -
