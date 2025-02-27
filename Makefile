objects := db.o err.o errsql.o escape.o hexdump.o progname.o

#EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Wimplicit-fallthrough -flto -fanalyzer -Og -ggdb

LDLIBS  += -lpthread
CFLAGS  += -std=gnu99 -Os -ggdb

.PHONY: all
all:	index.html index-with-ids.html DIGESTS.txt sql.txt hw.html

.PHONY: clean
clean:
	rm -f checkdb checkdb.o mkcds mkhw mkcds.o mkhw.o $(objects)

.PHONY: upload
upload:	index.html index-with-ids.html DIGESTS.txt sql.txt hw.html styles.css
	./checkdb -f sgi.db
	rsync -aPHAXz $^ jrrazone:/www/jrra.zone/sgi/

mkcds: mkcds.o $(objects) sqlite3.o

mkhw: mkhw.o $(objects) sqlite3.o

checkdb: checkdb.o $(objects) sqlite3.o

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
