objects := db.o errsql.o escape.o

#EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Wimplicit-fallthrough -flto -fanalyzer -Og -ggdb

LDFLAGS += -lsqlite3
CFLAGS  += -std=c99

.PHONY: all
all:	index.html index-with-ids.html DIGESTS.txt sql.txt hw.html

.PHONY: clean
clean:
	rm -f mkcds mkhw mkcds.o mkhw.o $(objects) index.html index-with-ids.html sql.txt hw.html

.PHONY: upload
upload:	index.html index-with-ids.html DIGESTS.txt sql.txt hw.html styles.css
	rsync -aPHAXz $^ jrrazone:/www/jrra.zone/sgi/

mkcds: mkcds.o $(objects)

mkhw: mkhw.o $(objects)

index.html: mkcds sgi.db
	./mkcds -f sgi.db | xmllint --valid --output $@ -

index-with-ids.html: mkcds sgi.db
	./mkcds -f sgi.db -i -h | xmllint --valid --output $@ -

DIGESTS.txt: mkcds sgi.db mkdigests.sql
	sqlite3 sgi.db < mkdigests.sql > $@

sql.txt: sgi.db
	sqlite3 sgi.db ".dump" > sql.txt

hw.html: mkhw sgi.db
	./mkhw -f sgi.db | xmllint --valid --output $@ -
