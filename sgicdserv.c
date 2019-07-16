#define _GNU_SOURCE
#include <iso646.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ulfius.h>
#include "sa.h"

#define PORT 8081
#define DB_FILENAME "sgicds.db"

sqlite3 *db;

char *DB_GetFilenameForDisc(int disc_id)
{
	__label__ out_ok, out_err;
	int rc;
	sqlite3_stmt *stmt;
	char *zOut = NULL;
	rc = sqlite3_prepare_v2(
		db,
		"select name from discs where disc_id==?;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_bind_int(stmt, 1, disc_id);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) goto out_err;

	const char *temp;
	temp = sqlite3_column_text(stmt, 0);
	if (temp == NULL) goto out_err;
	zOut = strdup(temp);
	goto out_ok;
	
out_ok:
	sqlite3_finalize(stmt);
	return zOut;
out_err:
	sqlite3_finalize(stmt);
	return NULL;
}

int DB_GetNumDiscsForProduct(int product_id)
{
	__label__ out_ok, out_err;
	int rc;
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(
		db,
		"select count(*) from discs where product_id==?;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_bind_int(stmt, 1, product_id);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) goto out_err;

	rc = sqlite3_column_int(stmt, 0);
	goto out_ok;

out_ok:
	sqlite3_finalize(stmt);
	return rc;
out_err:
	sqlite3_finalize(stmt);
	return -rc;
}

char *DB_GetNameForProduct(int product_id)
{
	__label__ out_ok, out_err;
	int rc;
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(
		db,
		"select name from products where product_id==?;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_bind_int(stmt, 1, product_id);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) goto out_err;

	char *zOut = strdup(sqlite3_column_text(stmt, 0));
	goto out_ok;

out_ok:
	sqlite3_finalize(stmt);
	return zOut;
out_err:
	sqlite3_finalize(stmt);
	return NULL;
}

void make_discs(struct _string_array *sa, int product_id)
{
	int rc;


	sqlite3_stmt *stmt_discs;
	rc = sqlite3_prepare_v2(
		db,
		"select name, cd_pn, note, fromjrra, date, filename, disc_id from discs where product_id==? order by ordinal, name, date;",
		-1,
		&stmt_discs,
		NULL
	);

	sqlite3_bind_int(stmt_discs, 1, product_id);
	int num_discs = DB_GetNumDiscsForProduct(product_id);
	bool did_first_row = false;
	while ((rc = sqlite3_step(stmt_discs)) == SQLITE_ROW) {
		const char *name = sqlite3_column_text(stmt_discs, 0);
		const char *cd_pn = sqlite3_column_text(stmt_discs, 1);
		const char *note = sqlite3_column_text(stmt_discs, 2);
		int fromjrra = sqlite3_column_int(stmt_discs, 3);
		const char *date = sqlite3_column_text(stmt_discs, 4);
		const char *filename = sqlite3_column_text(stmt_discs, 5);
		int disc_id = sqlite3_column_int(stmt_discs, 6);
		_sa_add(sa, "<tr>\n");
		if (!did_first_row) {
			did_first_row = true;

			char *s;
			asprintf(&s, "\t<td rowspan='%d'>", num_discs);
			_sa_add(sa, s);
			free(s);
			s = DB_GetNameForProduct(product_id);
			_sa_add(sa, s);
			free(s);
			_sa_add(sa, "</td>\n");
		}
		_sa_add(sa, "\t<td>");
		_sa_add(sa, cd_pn);
		_sa_add(sa, "<br />");
		_sa_add(sa, date);
		_sa_add(sa, "</td>\n");
		_sa_add(sa, "\t<td>");
/*
		char *s;
		asprintf(&s, "%d ", disc_id);
		_sa_add(sa, s);
		free(s);
*/
		_sa_add(sa, "<a href=\"");
		_sa_add(sa, filename);
		_sa_add(sa, "\">");
		_sa_add(sa, name);
		_sa_add(sa, "</a>");
		if (!fromjrra) {
			_sa_add(sa, "<br /><span class='contrib'>contributed</span>");
		}
		if (note && strlen(note) > 0) {
			_sa_add(sa, "<br /><span class='note'>note</span>: ");
			_sa_add(sa, note);
		}
		_sa_add(sa, "</td>\n");
		_sa_add(sa, "</tr>\n");
	}
	sqlite3_finalize(stmt_discs);
}

void make_products(struct _string_array *sa, int pg_id)
{
	int rc;


	sqlite3_stmt *stmt_products;
	rc = sqlite3_prepare_v2(
		db,
		"select product_id, name from products where product_group_id==? order by ordinal, name;",
		-1,
		&stmt_products,
		NULL
	);

	sqlite3_bind_int(stmt_products, 1, pg_id);
	while ((rc = sqlite3_step(stmt_products)) == SQLITE_ROW) {
		int product_id = sqlite3_column_int(stmt_products, 0);
		const char *name = sqlite3_column_text(stmt_products, 1);

		make_discs(sa, product_id);
	}
	sqlite3_finalize(stmt_products);
}

int callback_sgi_cds (
	const struct _u_request *request,
	struct _u_response *response,
	void *db
){
	__label__ out_200, out_500, out_finalize, out_return;
	int rc;
	struct _string_array sa;

	_sa_init(&sa);
	_sa_add(&sa,
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
		".note {font-weight: bold; color: red; text-decoration: underline;}\n"
		".contrib {font-weight: bold; color: green; text-decoration: underline;}\n"
		"@font-face {font-family: 'FatFrank Heavy';src: url('/FatFrank-Heavy.eot');src:	url('/FatFrank-Heavy.eot?#iefix') format('embedded-opentype'),url('/FatFrank-Heavy.woff') format('woff'),url('/FatFrank-Heavy.ttf') format('truetype');font-style: normal;font-weight: 400;}\n"
		"body,h1,h2,h3 {font-family: Helvetica;}\n"
		".header {border-bottom: 0.25em solid #ff4081;text-decoration:none;display:inline;}\n"
		".header h1 {font-family: 'FatFrank Heavy';font-size: 43px;font-weight: normal;color:#3f51b5;display:inline;text-shadow: 2px 2px 0 white, -2px 2px 0 white;}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<div class=\"header\"><h1>jrra.zone</h1></div><hr />\n"
		"<h2>SGI/IRIX CDs</h2>\n"
	);

	sqlite3_stmt *stmt_product_groups = NULL;
	rc = sqlite3_prepare_v2(
		db,
		"select product_group_id, name from product_groups order by ordinal, name;",
		-1,
		&stmt_product_groups,
		NULL
	);
	if (rc != SQLITE_OK) {
		sqlite3_finalize(stmt_product_groups);
		_sa_free(&sa);
		fprintf(stderr, "couldn't prepare statement\n");
		goto out_500;
	}



	for(;;) switch (rc = sqlite3_step(stmt_product_groups)) {
	case SQLITE_ROW:
		{
		int product_group_id = sqlite3_column_int(stmt_product_groups, 0);
		const char *pg_name = sqlite3_column_text(stmt_product_groups, 1);
		_sa_add(&sa, "<table>\n<caption>");
		_sa_add(&sa, pg_name);
		_sa_add(&sa, "</caption>\n<thead>\n<tr>\n");
		_sa_add(&sa, "\t<th scope='col'>product</th>\n");
		_sa_add(&sa, "\t<th scope='col'>cd pn</th>\n");
		_sa_add(&sa, "\t<th scope='col'>title</th>\n");
		_sa_add(&sa, "</tr>\n</thead>\n<tbody>\n");
		make_products(&sa, product_group_id);
		_sa_add(&sa, "</tbody></table>\n");
		}
		break;
	case SQLITE_DONE:
		_sa_add(&sa, "</body></html>");
		goto out_200;
		break;
	default:
		goto out_500;
		break;
	}

out_200:
	response->status = 200;
	response->binary_body = _sa_get(&sa);
	response->binary_body_length = strlen(response->binary_body);
	ulfius_add_header_to_response(response, "Content-Type", "application/xhtml+xml; charset=utf-8");
	goto out_finalize;
out_500:
	ulfius_set_string_body_response(response, 500, "woops");
out_finalize:
	_sa_free(&sa);
	sqlite3_finalize(stmt_product_groups);
out_return:
	return U_CALLBACK_CONTINUE;
}

int main(int argc, char *argv[])
{
	__label__ out_close_db, out_clean_ulfius, out_stop_framework;
	int rc;
	char *err = NULL;
	int returnval = EXIT_FAILURE;

	rc = sqlite3_open_v2(DB_FILENAME, &db, SQLITE_OPEN_READONLY, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n",
			sqlite3_errmsg(db));
		goto out_close_db;
	}

	struct _u_instance ulfius;
	rc = ulfius_init_instance(&ulfius, PORT, NULL, NULL);
	if (rc != U_OK) {
		fprintf(stderr, "ulfius_init_instance failed\n");
		goto out_clean_ulfius;
	}

	ulfius_add_endpoint_by_val(
		&ulfius,
		"GET",
		"/",
		NULL,
		0,
		&callback_sgi_cds,
		db
	);

	rc = ulfius_start_framework(&ulfius);
	if (rc != U_OK) {
		fprintf(stderr, "ulfius_start_framework failed\n");
		goto out_clean_ulfius;
	}

	printf("Press any key to quit.\n");
	getchar();

	returnval = EXIT_SUCCESS;

out_stop_framework:
	ulfius_stop_framework(&ulfius);
out_clean_ulfius:
	ulfius_clean_instance(&ulfius);
out_close_db:
	sqlite3_close(db);
	return returnval;
}

