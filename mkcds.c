#define _GNU_SOURCE
#include <iso646.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "db.h"
#include "escape.h"
#include "stdnoreturn.h"

extern char *__progname;
static void noreturn usage(void);

static bool show_ids = false;
static bool show_hashes = false;

const char webroot[] = "/sgi";

void make_discs(struct product_s product)
{
	struct disc_s disc;
	bool did_first_row = false;
	foreachdisc(disc, product.id) {
		char *name = escape_xml(disc.name);
		char *note = NULL;
		if (disc.note)
			note = escape_xml(disc.note);
		char *attachmentURL = NULL;
		char *attachmentXML = NULL;
		if (disc.havefile && disc.attachment) {
			attachmentURL = escape_url(disc.attachment);
			attachmentXML = escape_xml(disc.attachment);
		}
		char *filename;
		if (disc.filename and disc.havefile) {
			filename = escape_url(disc.filename);
		} else if (disc.havefile) {
			filename = escape_url(disc.name);
			char *nf = malloc(strlen(filename) + strlen(".iso") + 1);
			strcpy(nf, filename);
			strcat(nf, ".iso");
			free(filename);
			filename = nf;
		} else {
			filename = strdup("");
		}

		char *tarname = NULL;
		if (disc.havetar) {
			tarname = strdup(filename);
			char *dot = rindex(tarname, '.');
			if (dot and (strlen(dot) >= strlen(".tar")))
				strcpy(dot, ".tar");
		}

		printf("<tr id=\"disc-%d\">\n", disc.id);
		if (!did_first_row) {
			did_first_row = true;

			printf( "\t<td rowspan='%d'>", product.num_discs);
			if (show_ids) {
				printf("%d ", disc.product_id);
			}
			printf("%s", product.name);
			printf("</td>\n");
		}
		printf("\t<td>");
		if (disc.cd_pn) {
			printf("%s", disc.cd_pn);
		} else {
			printf("&nbsp;");
		}
		printf("<br />");
		if (disc.date) {
			printf("%s", disc.date);
		} else {
			printf("&nbsp;");
		}
		printf("</td>\n");
		printf("\t<td>");
		if (show_ids) {
			printf("%d ", disc.id);
		}
		if (disc.havefile) {
			printf("<a href=\"%s/cds/%s\">%s</a>", webroot, filename, name);
		} else {
			printf(name);
		}
		if (disc.havetar) {
			printf(" (<a href=\"%s/tar/%s\">tar</a>)", webroot, tarname);
		}
		if (disc.is_newest) {
			printf(" <span class=\"newest\">new</span>");
		}
		if (disc.contributor and strcmp(disc.contributor,"jrra")) {
			printf("<br /><span class='contrib'>contributed by ");
			printf("%s", disc.contributor);
			printf("</span>");
		}
		if (note) {
			printf("<br />");
			printf(note);
		}
		if (show_hashes) {
			if (disc.md5)    printf("<br />MD5: %s\n", disc.md5);
			if (disc.sha1)   printf("<br />SHA1: %s\n", disc.sha1);
			if (disc.sha256) printf("<br />SHA256: %s\n", disc.sha256);
			if (disc.bsdsum) printf("<br />sum -r: %s\n", disc.bsdsum);
		}
		free(disc.md5);
		free(disc.sha1);
		free(disc.sha256);
		free(disc.bsdsum);
		disc.md5 = disc.sha1 = disc.sha256 = disc.bsdsum = NULL;

		if (attachmentURL && attachmentXML) {
			printf("<br /><span class='attachment'>attachment: ");
			printf("<a href=\"%s/cds/%s\">%s</a></span>", webroot, attachmentURL, attachmentXML);
		}
		printf("</td>\n");
		printf("</tr>\n");
		free(filename);
		free(tarname);
		free(name);
		free(note);
		free(attachmentURL);
		free(attachmentXML);
	}
}

void make_products(int pg_id)
{
	struct product_s product;
	foreachproduct(product, pg_id) {
		make_discs(product);
	}
}

int callback_sgi_cds()
{
	__label__ out_finalize, out_return;

	printf("%s",
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
		"<head>"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
		"<title>jrra.zone: SGI/IRIX CDs</title>\n"
		"<link rel=\"stylesheet\" href=\"styles.css\" type=\"text/css\" media=\"all\"/>\n"
		"</head>\n"
		"<body>\n"
		"<h1><a href=\"/\"><img src=\"../jrrazone.svg\" alt=\"jrra.zone\"/></a></h1>\n"
		"<h2>SGI/IRIX CDs</h2>\n"
		"<hr/>\n"
		"<h3>News</h3>\n"
		"<p>2024-12-04: Added a few more discs courtesy of VoxAndrews. Thank you!</p>\n"
		"<p>2024-12-03: Added DANG Rev C Patches courtesy of VoxAndrews. Thank you!</p>\n"
		"<p>2024-11-02: Added many more discs courtesy of chulofiasco, from the desk of bipin. Thank you both!</p>\n"
		"<p>2024-10-29: Added Infinite Reality Demos (June 1996), courtesy of chulofiasco, from the desk of bipin. Thank you both!</p>\n"
		"<p>2024-10-28: Added a ton of Developers Toolbox discs, courtesy of chulofiasco. Thank you!</p>\n"
		"<p>2024-10-27: Added the 3-disc companion CD set for the SGI Developer Forum '95, courtesy of chulofiasco. Thank you!</p>\n"
		"<p>2024-05-05: Added a number of new discs courtesy of CB_HK, including several Hot Mixes. Thank you!</p>\n"
		"<p>2024-03-15: Added PDF Generator 1.2, courtesy of WildOne69. Thank you!</p>\n"
                "<p>2023-11-26: Regenerated all tars with <a href=\"/efsextract\">efsextract</a>, and added tars for discs which did not previously have them.</p>\n"
		"<p>2023-11-03: Added some discs from my own collection that needed redumps. Two versions each of IRIX 4.0.2 and IRIS Development Option 4.0 are now available, along with another ProPack.</p>\n"
		"<p>2023-10-26: Added lots of new discs, courtesy of whimsicalwilson. Thank you!</p>\n"
		"<p>2023-08-30: Added a ton of new discs, courtesy of plamen. Thank you!</p>\n"
		"<p>2023-08-20: Added a number of new discs, courtesy of twylo. Thanks!</p>\n"
		"<p>2023-08-05: Added IRIX 6.5.22 and IRIX 6.5.25, both courtesy of twylo. Thank you!</p>\n"
		"<p>2023-07-08: Added Trusted IRIX 5.3, from twylo. Thank you!</p>\n"
		"<p>2023-03-20: Added new discs from darkhelmet: Fortran 77 Compiler 3.5 and ImageVision Library 1.0. Thank you!</p>\n"
		"<p>2023-01-27: Added hashes/digest for all discs in HTML format: <a href=\"index-with-ids.html\">index-with-ids.html</a></p>\n"
		"<p>2023-01-09: Added hashes/digest for all discs: <a href=\"DIGESTS.txt\">DIGESTS.txt</a></p>\n"
		"<p>2022-12-14: Added a number of discs from chulofiasco, just in time for Christmas. Thank you!</p>\n"
		"<p>2022-11-25: Added two discs from Titox: 1600SW Flat Panel Patches for O2 on IRIX 6.3, and Fuel customer diagnostics. Thanks!</p>\n"
		"<p>2022-10-05: Added 21 new discs from Docter60. Thank you!</p>\n"
		"<p>2022-07-10: Added WorldView Europe 6.2.</p>\n"
		"<p>2022-06-27: Added IRIX 6.2/6.3/6.4 Development Foundation 1.1, and MIPSpro C Compiler 7.2.1, both courtesy of chulofiasco. Thanks!</p>\n"
		"<p>2022-06-08: Added a number of IRIX 6.3 discs. Thanks gijoe77!</p>\n"
		"<p>2022-06-07: Added IRIX 6.3 Recommended/Required Patches July 1998, and SoftWindows 95 4.0.1, both courtesy of chulofiasco. Thanks!</p>\n"
		"<p>2022-04-07: Added IRIX 6.3 with Applications, courtest of plamen. Thanks!</p>\n"
		"<p>2022-04-04: Added European Language Module 1.2.</p>\n"
		"<p>2022-04-02: Added NetVisualyzer Data and Display Station 2.2, courtesy of Ciao! Also added IRIX 6.5.30 plus some IRIX 5.3 patch discs courtesy of plamen. Also added IRIX 6.5.23 and WebFORCE August 1996.</p>\n"
		"<p>2022-02-08: Added WorldView Japanese 6.2A.</p>\n"
		"<p>2022-02-27: Added IRIX 6.5.26 courtesy of Jenna16bit. Thank you!</p>\n"
		"<p>2022-02-21: Added the Windows section, along with VW320 / VW540 system software from TAL. Thanks!</p>\n"
		"<p>2022-01-26: Added IRIX 6.5.14 and 6.5.15, plus 6.5.4 base documentation and Open Inventor 2.1.5, all courtesy of chulofiasco. Thank you!</p>\n"
		"<p>2022-01-23: Added IRIX 6.5.29 courtesy of chulofiasco. Thank you! Also added some missing discs from the 6.5.27 set.</p>\n"
		"<p>2021-09-15: Added tons of Itanium Linux discs courtesy of Titox. Thank you! Also added Pascal Compiler versions 1.3.1, 1.4.3, and 1.4.4.</p>\n"
		"<p>2021-08-26: Added MIPSpro 7.4 compilers for C, C++, and Fortran 77, along with Compiler Execution Environment 7.4, ProDev WorkShop 2.9.2, IRIX Development Foundation 1.3, and ProPack 6 SP3, all from chulofiasco via the library of Zerolapse. Thank you!</p>\n"
		"<p>2021-07-31: Added tarball downloads via Sophie Haskins' <a href=\"https://github.com/sophaskins/efs2tar\">efs2tar</a>. Thanks!</p>\n"
	);

	struct pg_s pg;
	struct product_s product;
	struct disc_s disc;

	printf("<h3>Index</h3>\n");
	printf("<ul>\n");
	foreachpg(pg) {
		printf("\t<li><a href=\"#pg-%d\">%s</a>", pg.id, pg.name);
		/*
		printf("\t<ul>\n");
		foreachproduct(product,pg.id) {
			printf("\t\t<li>%s</li>\n", product.name);
		}
		printf("\t</ul></li>\n");
		*/
		printf("</li>\n");
	}
	printf("</ul>\n");
	printf("<hr/>\n");

	foreachpg(pg) {
		printf("<table class=\"cd\" id=\"pg-%d\">\n<caption>", pg.id);
		printf("%s", pg.name);
		printf("</caption>\n<thead>\n<tr>\n");
		printf("\t<th scope='col'>product</th>\n");
		printf("\t<th scope='col'>cd pn<br />date</th>\n");
		printf("\t<th scope='col'>title</th>\n");
		printf("</tr>\n</thead>\n<tbody>\n");
		make_products(pg.id);
		printf("</tbody></table>\n");
	}
	printf("</body></html>");

out_finalize:
out_return:
	return 0;
}

int main(int argc, char *argv[])
{
	char *dbfilename = NULL;
	int rc;

	while ((rc = getopt(argc, argv, "f:ih")) != -1)
		switch (rc) {
		case 'f':
			if (dbfilename)
				usage();
			dbfilename = optarg;
			break;
		case 'i':
			if (show_ids)
				usage();
			show_ids = true;
			break;
		case 'h':
			if (show_hashes)
				usage();
			show_hashes = true;
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
	callback_sgi_cds();
	DB_Close();
	return 0;
}

static void noreturn usage()
{
	fprintf(stderr, "usage: %s -f dbfile [-i] [-h]\n", __progname);
	exit(1);
}
