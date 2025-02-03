#ifndef _HEXDUMP_H
#define _HEXDUMP_H
#include <stdint.h>
#include <stddef.h>
void hexdump(const void *buf, size_t siz);
void hexdump2(const void *buf, size_t siz, size_t off);
void hexdump_color(const void *buf, size_t siz);
#endif
