#define _GNU_SOURCE
#include <arpa/inet.h>
#include <iso646.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ulfius.h>

#define PORT 8081
#define DB_FILENAME "sgicds.db"

struct _string_array {
	size_t num_strings;
	size_t total_len;
	char **strings;
};

bool _sa_init (struct _string_array *sa)
{
	sa->num_strings = 0;
	sa->total_len = 0;
	sa->strings = malloc(0);
	return true;
}

void _sa_free (struct _string_array *sa)
{
	for (size_t i = 0; i < sa->num_strings; i++) {
		free(sa->strings[i]);
	}
	free(sa->strings);
}

bool _sa_add (struct _string_array *sa, const char *s)
{
	int rc;
	size_t new_num_strings = sa->num_strings + 1;

	char **temp = reallocarray(sa->strings, new_num_strings,
							sizeof(char *));
	if (temp == NULL) return false;

	sa->strings = temp;

	char *newstring = strdup(s);
	sa->strings[new_num_strings - 1] = newstring;

	size_t new_total_len = sa->total_len + strlen(newstring);
	sa->num_strings = new_num_strings;
	sa->total_len = new_total_len;
	return true;
}

char *_sa_get (struct _string_array *sa)
{
	char *t = malloc(sa->total_len + 1);
	char *p = t;
	for (size_t i = 0; i < sa->num_strings; i++) {
		size_t len = strlen(sa->strings[i]);
		memcpy(p, sa->strings[i], len);
		p += len;
	}
	p[0] = '\0';
	return t;
}

int callback_sgi_cds (
	const struct _u_request *request,
	struct _u_response *response,
	void *db
){
	__label__ out_200, out_500, out_finalize, out_return;
	int rc;

	sqlite3_stmt *stmt_list_cds = NULL;
	if (!stmt_list_cds) rc = sqlite3_prepare_v2(
		db,
		"SELECT cd_pn, strftime(\"%Y-%m\", date), title, note FROM discs order by title, date;",
		-1,
		&stmt_list_cds,
		NULL
	);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "couldn't prepare statement\n");
		goto out_return;
	}

	struct _string_array sa;
	_sa_init(&sa);

	_sa_add(&sa,
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
	"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
	"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" "
	"xml:lang=\"en\"><head><meta http-equiv="
	"\"content-type\" content="
	"\"text/html;charset=utf-8\" /><title>listing</title>"
	"<style type=\"text/css\">"
	"body {font: 1.1em sans-serif;}"
	"table, td {border: 1px solid #ddd;}"
	"thead {background-color: #333; color: #fff; font-weight: bold;}"
	".note {font-weight: bold; color: red; text-decoration: underline;}"
	"</style>"
	"</head><body>"
	"<h1>SGI/IRIX CDs</h1><hr />\n"
	"<table><thead><tr>"
	"<th scope=\"col\">cd_pn</th>"
	"<th scope=\"col\">date</th>"
	"<th scope=\"col\">title</th>"
	"</tr></thead>"
	"<tbody>"
	);

	for(;;) switch (rc = sqlite3_step(stmt_list_cds)) {
	case SQLITE_ROW:
		{
		const char *cd_pn = sqlite3_column_text(stmt_list_cds,0);
		const char *date = sqlite3_column_text(stmt_list_cds,1);
		const char *title = sqlite3_column_text(stmt_list_cds,2);
		const char *note = sqlite3_column_text(stmt_list_cds,3);

		_sa_add(&sa, "<tr><td><a href=\"/sgi/cds/");
		_sa_add(&sa, cd_pn);
		_sa_add(&sa, ".iso\">");
		_sa_add(&sa, cd_pn);
		_sa_add(&sa, "</a></td><td>");
		_sa_add(&sa, date);
		_sa_add(&sa, "</td><td>");
		_sa_add(&sa, title);
		if (strlen(note) > 0) {
			_sa_add(&sa, "<span class='note' title=\"");
			_sa_add(&sa, note);
			_sa_add(&sa, "\">*</span>");
		}
		_sa_add(&sa, "</td></tr>");
		_sa_add(&sa, "\n");
		}
		break;
	case SQLITE_DONE:
		printf("done\n");
		_sa_add(&sa, "</tbody></table></body></html>");
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
	_sa_free(&sa);
	goto out_finalize;
out_500:
	ulfius_set_string_body_response(response, 500, "woops");
out_finalize:
	sqlite3_finalize(stmt_list_cds);
out_return:
	return U_CALLBACK_CONTINUE;
}

int main(int argc, char *argv[])
{
	__label__ out_close_db, out_clean_ulfius, out_stop_framework;
	int rc;
	char *err = NULL;
	int returnval = EXIT_FAILURE;
	sqlite3 *db;

	rc = sqlite3_open(DB_FILENAME, &db);
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

