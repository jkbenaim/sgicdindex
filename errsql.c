#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "errsql.h"
#include "stdnoreturn.h"

extern const char *__progname;

void vwarnsql(sqlite3 *db, const char *fmt, va_list args)
{
	fprintf(stderr, "%s: ", __progname);
	if (fmt) {
		vfprintf(stderr, fmt, args);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", sqlite3_errmsg(db));
}

void noreturn verrsql(int eval, sqlite3 *db, const char *fmt, va_list args)
{
	vwarnsql(db, fmt, args);
	exit(eval);
}

void warnsql(sqlite3 *db, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnsql(db, fmt, ap);
	va_end(ap);
}

void noreturn errsql(int eval, sqlite3 *db, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrsql(eval, db, fmt, ap);
	va_end(ap);
}

