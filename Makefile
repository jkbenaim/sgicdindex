target  ?= sgicdserv
objects := $(patsubst %.c,%.o,$(wildcard *.c))

libs:= sqlite3 gnutls libmicrohttpd libsystemd jansson

EXTRAS += -fsanitize=undefined -fsanitize=null -fcf-protection=full -fstack-protector-all -fstack-check -Wimplicit-fallthrough

ifdef libs
LDLIBS  += $(shell pkg-config --libs   ${libs})
CFLAGS  += $(shell pkg-config --cflags ${libs})
endif

LDFLAGS += -lm -pthread ${EXTRAS}
LDLIBS  += ulfius/src/libulfius.a yder/src/libyder.a orcania/src/liborcania.a
CFLAGS  += -std=gnu11 -Og -ggdb -Iulfius/include -Iorcania/include -Iyder/include ${EXTRAS}

.PHONY: all
all:	$(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)
	make -C orcania/src clean
	make -C yder/src clean
	make -C ulfius/src clean

$(target): orcania/src/liborcania.a yder/src/libyder.a ulfius/src/libulfius.a $(objects)


$(objects): orcania/src/liborcania.a yder/src/libyder.a ulfius/src/libulfius.a


orcania/src/liborcania.a:
	make -C orcania/src liborcania.a

yder/src/libyder.a: orcania/src/liborcania.a
	CFLAGS=-I`pwd`/orcania/include make -C yder/src libyder.a

ulfius/src/libulfius.a: orcania/src/liborcania.a yder/src/libyder.a
	CFLAGS="-I`pwd`/orcania/include -I`pwd`/yder/include" make -C ulfius/src libulfius.a
