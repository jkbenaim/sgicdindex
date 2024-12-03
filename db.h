#pragma once

// this is new shit, never used

#include <stdbool.h>
#include <sqlite3.h>

struct pg_s {
	int id;
	const char *name;
	sqlite3_stmt *_stmt;
	int _rc;
};

struct product_s {
	int id;
	int pg_id;
	const char *name;
	int num_discs;
	sqlite3_stmt *_stmt;
	int _rc;
};

struct disc_s {
	int id;
	int product_id;
	const char *name;
	const char *cd_pn;
	const char *case_pn;
	const char *date;
	const char *note;
	const char *filename;
	const char *contributor;
	const char *attachment;
	bool is_newest;
	bool havefile;
	bool havetar;
	sqlite3_stmt *_stmt;
	int _rc;
	char *md5;
	char *sha1;
	char *sha256;
	char *bsdsum;
};

struct part_s {
	const char *doc;
	unsigned page;
	const char *item;
	const char *rev;
	const char *desc;
	const char *uom;
	sqlite3_stmt *_stmt;
	int _rc;
};

#define foreachpg(a) pginit(&a);while (a._rc=pgstep(&a),(a._rc!=SQLITE_ROW)?sqlite3_finalize(a._stmt):0,a._rc==SQLITE_ROW)
#define foreachproduct(a,pg_id) productinit(&a,pg_id);while (a._rc=productstep(&a),(a._rc!=SQLITE_ROW)?sqlite3_finalize(a._stmt):0,a._rc==SQLITE_ROW)
#define foreachdisc(a,product_id) discinit(&a,product_id);while (a._rc=discstep(&a),(a._rc!=SQLITE_ROW)?sqlite3_finalize(a._stmt):0,a._rc==SQLITE_ROW)
#define foreachpart(a) partinit(&a);while (a._rc=partstep(&a),(a._rc!=SQLITE_ROW)?sqlite3_finalize(a._stmt):0,a._rc==SQLITE_ROW)

void DB_Init(const char *filename);
void DB_Close();
void pginit(struct pg_s *pg);
int pgstep(struct pg_s *pg);
void productinit(struct product_s *product, int pg_id);
int productstep(struct product_s *product);
void discinit(struct disc_s *disc, int product_id);
int discstep(struct disc_s *disc);
void partinit(struct part_s *part);
int partstep(struct part_s *part);
