#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "err.h"
#include "errsql.h"
#include "hexdump.h"
#include "stdnoreturn.h"

extern char *__progname;
static void noreturn usage(void);

const char cdsdir[] = "/bulk/sgi/cds";

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
	int dirfd;

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
		"select disc_id, name, filename, note, contributor, havefile, disposition from discs;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "in prepare discs");
	
	/* Open CDs directory. */
	rc = open(cdsdir, O_RDONLY);
	if (rc == -1) {
		err(1, "couldn't open directory '%s'", cdsdir);
	} else {
		dirfd = rc;
	}

	/* Loop over all discs in the DB. */
	while (rc = sqlite3_step(stmt), rc == SQLITE_ROW) {
		int disc_id, havefile;
		const unsigned char *name, *filename, *contributor, *note, *disposition;
		char *myfn;
		struct stat sb = {0,};

		disc_id = sqlite3_column_int(stmt, 0);
		name = sqlite3_column_text(stmt, 1);
		filename = sqlite3_column_text(stmt, 2);
		note = sqlite3_column_text(stmt, 3);
		contributor = sqlite3_column_text(stmt, 4);
		havefile = sqlite3_column_int(stmt, 5);
		disposition = sqlite3_column_text(stmt, 6);

		/* Verify that string fields are 7-bit clean. */
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

		/* Verify that the disposition is not null. */
		rc = sqlite3_column_type(stmt, 6);
		if (rc != SQLITE_TEXT) {
			clean = false;
			printf("disposition is null: (%d)\n", disc_id);
		}

		/* Verify that this disc's .iso file is present. */
		if (havefile) {
			if (filename) {
				myfn = strdup(filename);
			} else {
				myfn = malloc(strlen(name) + strlen(".iso") + 1);
				strcpy(myfn, name);
				strcat(myfn, ".iso");
			}
			rc = fstatat(dirfd, myfn, &sb, 0);
			if (rc == -1) {
				printf("file not found: \"%s\"\n", myfn);
			}
			free(myfn);
			myfn = NULL;
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
