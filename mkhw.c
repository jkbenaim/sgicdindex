#define _GNU_SOURCE
#include <iso646.h>
#include "sqlite3.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "db.h"
#include "escape.h"
#include "stdnoreturn.h"
#include "progname.h"

extern char *__progname;
static void noreturn usage();

void make_parts()
{
	struct part_s part;
	printf(
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
		"<head>"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
		"<title>jrra.zone: SGI Hardware Part Numbers</title>\n"
		"<link rel=\"stylesheet\" href=\"styles.css\" type=\"text/css\" media=\"all\"/>\n"
		"</head>\n"
		"<body>\n"
		"<h1><a href=\"/\"><img src=\"../jrrazone.svg\" alt=\"jrra.zone\"/></a></h1>\n"
		"<table class=\"hw\">\n"
		"<caption>SGI Hardware Part Numbers</caption>\n"
		"<thead>\n"
		"<tr>\n"
		"\t<th scope=\"col\">item</th>\n"
		"\t<th scope=\"col\">rev</th>\n"
		"\t<th scope=\"col\">desc</th>\n"
		"</tr>\n"
		"</thead>\n"
		"<tbody>\n"
	);
	foreachpart(part) {
		char *item;
		char *rev;
		char *desc;

		item = escape_xml(part.item?:"");
		rev = escape_xml(part.rev?:"");
		desc = escape_xml(part.desc?:"");

		printf("<tr>\n");
		printf("\t<td>%s</td>\n", item);
		printf("\t<td>%s</td>\n", rev);
		printf("\t<td>%s</td>\n", desc);
		printf("</tr>\n");

		free(item);
		free(rev);
		free(desc);
	}
	printf("</tbody>\n</table>\n");
	printf("</body></html>\n");
}

int main(int argc, char *argv[])
{
	char *dbfilename = NULL;
	int rc;

	progname_init(argc, argv);

	while ((rc = getopt(argc, argv, "f:")) != -1)
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
	if (not dbfilename)
		usage();
	
	DB_Init(dbfilename);
	make_parts();
	DB_Close();
	return 0;
}

static void noreturn usage()
{
	fprintf(stderr, "usage: %s -f dbfile\n", __progname);
	exit(1);
}
