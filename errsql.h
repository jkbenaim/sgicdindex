#pragma once

#include <stdarg.h>
#include "sqlite3.h"
#include "stdnoreturn.h"

void vwarnsql(sqlite3 *db, const char *fmt, va_list args);
void noreturn verrsql(int eval, sqlite3 *db, const char *fmt, va_list args);
void warnsql(sqlite3 *db, const char *fmt, ...);
void noreturn errsql(int eval, sqlite3 *db, const char *fmt, ...);

