#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "hexdump.h"

enum colors_e {
	COLOR_NONE = 0,
	COLOR_RED = 31,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE
};

void _changecol(enum colors_e color)
{
	static enum colors_e oldcolor = COLOR_NONE;

	if (color != oldcolor) {
		oldcolor = color;
		if (color)
			printf("\e[01;%dm", color);
		else
			printf("\e[00m");
	}
}

void _changecol_for_char(uint8_t c)
{
	switch (c) {
	case '\0':
		_changecol(COLOR_WHITE);
		break;
	case '\t':
		/* fall-thru */
	case '\n':
		/* fall-thru */
	case '\r':
		_changecol(COLOR_YELLOW);
		break;
	case 0x20 ... 0x7e:
		_changecol(COLOR_GREEN);
		break;
	case 0xff:
		_changecol(COLOR_BLUE);
		break;
	default:
		_changecol(COLOR_RED);
		break;
	}
}

void _hexdump_aux(const void *buf, size_t siz, size_t off, int color)
{
	const uint8_t *mybuf = buf;
	while (siz) {
		unsigned numbytes;
		size_t i;
		numbytes = (siz>16)?16:siz;

		_changecol(COLOR_NONE);
		printf("%08zx: ", off);

		for (i = 0; i < numbytes; i++) {
			_changecol_for_char(mybuf[i]);
			printf("%02x", mybuf[i]);
			if (i & 1) {
				_changecol(COLOR_NONE);
				printf(" ");
			}
		}
		_changecol(COLOR_NONE);
		for (i = numbytes; i < 16; i++) {
			if (i & 1) {
				printf("   ");
			} else {
				printf("  ");
			}
		}
		printf(" ");
		for (i = 0; i < numbytes; i++) {
			_changecol_for_char(mybuf[i]);
			putchar(isprint(mybuf[i])?mybuf[i]:'.');
		}
		_changecol(COLOR_NONE);
		printf("\n");
		siz -= numbytes;
		mybuf += numbytes;
		off += 16;
	}
}

void hexdump2(const void *buf, size_t siz, size_t off)
{
	_hexdump_aux(buf, siz, off, 0);
}

void hexdump_color(const void *buf, size_t siz)
{
	_hexdump_aux(buf, siz, 0, 1);
}

void hexdump(const void *buf, size_t siz)
{
	_hexdump_aux(buf, siz, 0, 0);
}
