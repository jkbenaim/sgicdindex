#define _GNU_SOURCE

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "db.h"
#include "err.h"
#include "errsql.h"

sqlite3 *db;

#ifdef sqlite3_prepare_v2
#define sqlite3_prepare_v2_maybe sqlite3_prepare_v2
#else
#define sqlite3_prepare_v2_maybe sqlite3_prepare
#endif

void DB_Init(const char *filename)
{
	int rc;
#ifdef sqlite3_open_v2
	rc = sqlite3_open_v2(
		filename,
		&db,
		SQLITE_OPEN_READONLY,
		NULL
	);
#else
	rc = sqlite3_open(
		filename,
		&db
	);
#endif
	if (rc != SQLITE_OK)
		errsql(1, db, "DB_Init open");
}

void DB_Close()
{
	int rc;

	rc = sqlite3_close(db);
	if (rc != SQLITE_OK)
		errsql(1, db, "DB_Close close");
}

void pginit(struct pg_s *pg)
{
	int rc;

	rc = sqlite3_prepare_v2_maybe(
		db,
		"select product_group_id, name from product_groups order by ordinal, name collate nocase;",
		-1,
		&(pg->_stmt),
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "pginit prepare");
}

int pgstep(struct pg_s *pg)
{
	int rc;
	rc = sqlite3_step(pg->_stmt);
	if (rc == SQLITE_ROW) {
		pg->id		= sqlite3_column_int(pg->_stmt, 0);
		pg->name	= sqlite3_column_text(pg->_stmt, 1);
	}
	return rc;
}


int DB_GetNumDiscsForProduct(int product_id)
{
	int rc;
	int count;
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2_maybe(
		db,
		"select count(*) from discs where (product_id + 0)==?",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "DB_GetNumDiscsForProduct prepare");

	rc = sqlite3_bind_int(stmt, 1, product_id);
	if (rc != SQLITE_OK)
		errsql(1, db, "DB_GetNumDiscsForProduct bind");
	
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW)
		errsql(1, db, "DB_GetNumDiscsForProduct step");
	
	count = sqlite3_column_int(stmt, 0);
	
	rc = sqlite3_finalize(stmt);
	if (rc != SQLITE_OK)
		errsql(1, db, "DB_GetNumDiscsForProduct finalize");
	
	return count;
}

char *hashfordisc(int disc_id, const char *hashtype)
{
	int rc;
	sqlite3_stmt *stmt;
	const char *col;
	char *hash = NULL;

	rc = sqlite3_prepare_v2_maybe(
		db,
		"SELECT hash FROM hashes WHERE disc_id=? AND hashtype=?",
		-1,
		&stmt,
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "hashfordisc prepare");

	rc = sqlite3_bind_int(stmt, 1, disc_id);
	if (rc != SQLITE_OK)
		errsql(1, db, "hashfordisc bind disc_id");

	rc = sqlite3_bind_text(stmt, 2, hashtype, -1, SQLITE_STATIC);
	if (rc != SQLITE_OK)
		errsql(1, db, "hashfordisc bind hashtype");

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			errsql(1, db, "hashfordisc notrow-finalize");
		return NULL;
	}

	col = sqlite3_column_text(stmt, 0);
	if (col)
		hash = strdup(col);

	rc = sqlite3_finalize(stmt);
	if (rc != SQLITE_OK)
		errsql(1, db, "hashfordisc finalize");

	return hash;
}

void productinit(struct product_s *product, int pg_id)
{
	int rc;
	product->pg_id = pg_id;
	rc = sqlite3_prepare_v2_maybe(
		db,
		"select product_id, name from products where (product_group_id + 0)==? order by ordinal, name collate nocase;",
		-1,
		&(product->_stmt),
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "productinit prepare");
	rc = sqlite3_bind_int(product->_stmt, 1, pg_id);
	if (rc != SQLITE_OK)
		errsql(1, db, "productinit bind");
}
int productstep(struct product_s *product)
{
	int rc;
	rc = sqlite3_step(product->_stmt);
	if (rc == SQLITE_ROW) {
		product->id	= sqlite3_column_int(product->_stmt, 0);
		product->name	= sqlite3_column_text(product->_stmt, 1);
		product->num_discs = DB_GetNumDiscsForProduct(product->id);
	}
	return rc;
}

void discinit(struct disc_s *disc, int product_id)
{
	int rc;
	disc->product_id = product_id;
	rc = sqlite3_prepare_v2_maybe(
		db,
		"select disc_id, name, cd_pn, case_pn, substr(date,6,2)||'/'||substr(date,1,4), note, filename, contributor, attachment, date_added=(select max(Date_added) from discs), havefile, havetar from discs where (product_id + 0)==? order by ordinal, name collate nocase, date, cd_pn;",
		-1,
		&(disc->_stmt),
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "discinit prepare");
	sqlite3_bind_int(disc->_stmt, 1, product_id);
	if (rc != SQLITE_OK)
		errsql(1, db, "discinit bind");
}

int discstep(struct disc_s *disc)
{
	int rc;
	rc = sqlite3_step(disc->_stmt);
	if (rc == SQLITE_ROW) {
		disc->id	= sqlite3_column_int(disc->_stmt, 0);
		disc->name	= sqlite3_column_text(disc->_stmt, 1);
		disc->cd_pn	= sqlite3_column_text(disc->_stmt, 2);
		disc->case_pn	= sqlite3_column_text(disc->_stmt, 3);
		disc->date	= sqlite3_column_text(disc->_stmt, 4);
		disc->note	= sqlite3_column_text(disc->_stmt, 5);
		disc->filename	= sqlite3_column_text(disc->_stmt, 6);
		disc->contributor= sqlite3_column_text(disc->_stmt, 7);
		disc->attachment= sqlite3_column_text(disc->_stmt, 8);
		disc->is_newest	= sqlite3_column_int(disc->_stmt, 9);
		disc->havefile	= sqlite3_column_int(disc->_stmt, 10);
		disc->havetar   = sqlite3_column_int(disc->_stmt, 11);
		disc->md5       = hashfordisc(disc->id, "MD5");
		disc->sha1      = hashfordisc(disc->id, "SHA1");
		disc->sha256    = hashfordisc(disc->id, "SHA256");
		disc->bsdsum    = hashfordisc(disc->id, "BSDSUM");
	}
	return rc;
}

void partinit(struct part_s *part)
{
	int rc;
	rc = sqlite3_prepare_v2_maybe(
		db,
		"select doc,page,item,rev,desc,uom,quantity from parts group by item,rev order by item,rev;",
		-1,
		&(part->_stmt),
		NULL
	);
	if (rc != SQLITE_OK)
		errsql(1, db, "partinit prepare");
}

int partstep(struct part_s *part)
{
	int rc;
	rc = sqlite3_step(part->_stmt);
	if (rc == SQLITE_ROW) {
		part->doc  = sqlite3_column_text(part->_stmt, 0);
		part->page = sqlite3_column_int(part->_stmt, 1);
		part->item = sqlite3_column_text(part->_stmt, 2);
		part->rev  = sqlite3_column_text(part->_stmt, 3);
		part->desc = sqlite3_column_text(part->_stmt, 4);
		part->uom  = sqlite3_column_text(part->_stmt, 5);
	}
	return rc;
}
