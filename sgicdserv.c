#define _GNU_SOURCE
#include <arpa/inet.h>
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

int DB_GetNumProductsForGroup(int pg_id)
{
	__label__ out_err;
	int rc;
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(
		db,
		"select count(*) from products where product_group_id==?;",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK) goto out_err;

	rc = sqlite3_bind_int(stmt, 1, pg_id);
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
		"table {border: 1px solid #ddd; border-collapse: collapse;}\n"
		"caption {background-color: grey; color: white;font: italic bold 1.5em sans-serif;padding:10px;}\n"
		"td {border: 1px solid #ddd; font: 1em monospace;padding:20px;}\n"
		"th {font: 1em monospace;}\n"
		".note {font-weight: bold; color: red; text-decoration: underline;}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<h1>jrra.zone: SGI/IRIX CDs</h1><hr />\n"
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

	void make_product(int product_group_id)
	{
		sqlite3_stmt *stmt_products;
		rc = sqlite3_prepare_v2(
			db,
			"select count(*) from products where product_group_id==?;",
			-1,
			&stmt_products,
			NULL
		);
		sqlite3_bind_int(stmt_products, 1, product_group_id);
		sqlite3_step(stmt_products);
		int num_cds = sqlite3_column_int(stmt_products, 0);
		sqlite3_finalize(stmt_products);

		rc = sqlite3_prepare_v2(
			db,
			"select product_id, name from products where product_group_id==? order by ordinal, name;",
			-1,
			&stmt_products,
			NULL
		);

		sqlite3_bind_int(stmt_products, 1, product_group_id);
		int rc;
		while ((rc = sqlite3_step(stmt_products)) == SQLITE_ROW) {
			int product_id = sqlite3_column_int(stmt_products, 0);
			const char *name = sqlite3_column_text(stmt_products, 1);
			_sa_add(&sa, "<tr>\n");
			_sa_add(&sa, "</tr>\n");

		}
		sqlite3_finalize(stmt_products);
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
		_sa_add(&sa, "\t<th scope='col'>disc pn</th>\n");
		_sa_add(&sa, "\t<th scope='col'>disc title</th>\n");
		_sa_add(&sa, "</tr>\n</thead>\n<tbody>\n");
		make_product(product_group_id);
		_sa_add(&sa, "</tbody></table>\n<br />\n");



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
		"/sgi/cds/",
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

