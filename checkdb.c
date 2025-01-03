#define _GNU_SOURCE
#include <ctype.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdnoreturn.h"
#include <unistd.h>
#include <string.h>
#include "errsql.h"
#include "hexdump.h"

extern char *__progname;
static void noreturn usage(void);

bool is_7bit_clean(const unsigned char *t)
{
	if (!t) return true;

	while(*t) {
		unsigned char c = *t++;
		if (!isprint(c)) {
			return false;
		}
	}
	return true;
}

int main(int argc, char *argv[])
{
	char *dbfilename = NULL;
	int rc;
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	bool clean = true;

	while ((rc = getopt(argc, argv, "f:h")) != -1)
		switch (rc) {
		case 'f':
			if (dbfilename)
				usage();
			dbfilename = optarg;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (*argv != NULL)
		usage();
	if (!dbfilename)
		usage();
	
	rc = sqlite3_open_v2(
		dbfilename,
		&db,
		SQLITE_OPEN_READONLY,
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "couldn't open db");
	
	rc = sqlite3_prepare_v2(
		db,
		"select disc_id, name, filename, note, contributor from discs;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "in prepare discs");
	
	while (rc = sqlite3_step(stmt), rc == SQLITE_ROW) {
		int disc_id;
		const unsigned char *name, *filename, *contributor, *note;
		disc_id = sqlite3_column_int(stmt, 0);
		name = sqlite3_column_text(stmt, 1);
		filename = sqlite3_column_text(stmt, 2);
		contributor = sqlite3_column_text(stmt, 3);
		if (!is_7bit_clean(name)) {
			clean = false;
			printf("unclean name: (%d) '%s'\n", disc_id, name);
			hexdump(name, strlen(name));
			printf("\n");
		}
		if (!is_7bit_clean(filename)) {
			clean = false;
			printf("unclean filename: (%d) '%s'\n", disc_id, filename);
			hexdump(filename, strlen(filename));
			printf("\n");
		}
		if (!is_7bit_clean(contributor)) {
			clean = false;
			printf("unclean contributor: (%d) '%s'\n", disc_id, contributor);
			hexdump(contributor, strlen(contributor));
			printf("\n");
		}
		if (!is_7bit_clean(note)) {
			clean = false;
			printf("unclean note: (%d) '%s'\n", disc_id, note);
			hexdump(note, strlen(note));
			printf("\n");
		}
	}
	if (rc != SQLITE_DONE)
		errsql(1, db, "in discs step");
	
	rc = sqlite3_finalize(stmt);
	if (rc != SQLITE_OK)
		errsql(1, db, "in discs finalize");

	stmt = NULL;
	rc = sqlite3_close(db);
	if (rc != SQLITE_OK)
		errsql(1, db, "in db close");
	
	if (clean)
		return 0;
	else
		return 1;
}

static void noreturn usage(void)
{
	fprintf(stderr, "usage: %s -f dbfile\n", __progname);
	exit(1);
}
