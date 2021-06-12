#define _GNU_SOURCE
#include <iso646.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include "db.h"
#include "escape.h"

extern char *__progname;
static void noreturn usage(void);

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
		if (disc.attachment) {
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

		printf("<tr id=\"disc-%d\">\n", disc.id);
		if (!did_first_row) {
			did_first_row = true;

			printf( "\t<td rowspan='%d'>", product.num_discs);
#ifdef SHOW_IDS
			printf("%d ", disc.product_id);
#endif
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
#ifdef SHOW_IDS
		printf("%d ", disc.id);
#endif
		if (disc.havefile) {
			printf("<a href=\"cds/");
			printf("%s", filename);
			printf("\">");
			printf("%s", name);
			printf("</a>");
		} else {
			printf(disc.name);
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
		if (attachmentURL && attachmentXML) {
			printf("<br /><span class='attachment'>attachment: ");
			printf("<a href=\"cds/");
			printf(attachmentURL);
			printf("\">");
			printf(attachmentXML);
			printf("</a></span>");
		}
		printf("</td>\n");
		printf("</tr>\n");
		free(filename);
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
	int rc;

	printf("%s",
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
		"<head>\n"
		"<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />\n"
		"<title>jrra.zone: SGI/IRIX CDs</title>\n"
		"<style type=\"text/css\">\n"
		"body {font: 1.1em sans-serif;}\n"
		"table {border: 1px solid gray; border-collapse: collapse;table-layout:fixed;width:800px;margin-bottom:1em;}\n"
		"caption {background-color: grey; color: white;font: italic bold 1.5em sans-serif;padding:10px;}\n"
		"td {border: 1px solid gray; font: 1em monospace;padding:10px;}\n"
		"th {font: 1em monospace;}\n"
		"th:nth-child(1),th:nth-child(2) {width:150px;}\n"
		".newest {font-weight: bold; color: red; text-decoration: underline;}\n"
		".contrib {font-weight: bold; color: green; text-decoration: underline;}\n"
		"body,h1,h2,h3 {font-family: Helvetica;}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<h1><a href=\"/\"><img src=\"../jrrazone.svg\" alt=\"jrra.zone\"/></a></h1>\n"
		"<h2>SGI/IRIX CDs</h2>\n"
	);

	struct pg_s pg;
	struct product_s product;
	struct disc_s disc;

	printf("<hr/>\n");
	printf("<h3>Index</h3>\n");
	printf("<ul>\n");
	foreachpg(pg) {
		printf("\t<li><a href=\"#pg-%d\">%s</a>\n", pg.id, pg.name);
		printf("\t<ul>\n");
		foreachproduct(product,pg.id) {
			printf("\t\t<li>%s</li>\n", product.name);
		}
		printf("\t</ul></li>\n");
	}
	printf("</ul>\n");
	printf("<hr/>\n");

	foreachpg(pg) {
		printf("<table id=\"pg-%d\">\n<caption>", pg.id);
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
	if (argc < 2)
		usage();

	DB_Init(argv[1]);
	callback_sgi_cds();
	DB_Close();
	return 0;
}

static void noreturn usage()
{
	fprintf(stderr, "usage: %s dbfile\n", __progname);
	exit(1);
}
