#ifndef _SA_H_
#define _SA_H_
#include <stdbool.h>

struct _sa_string_s {
	char *s;
	void (*d)(void *);
};

struct _string_array {
	size_t slots_used;
	size_t total_len;
	size_t slots_available;
	struct _sa_string_s *strings;
};

size_t _sa_next(size_t old) __attribute__((pure));
bool _sa_init (struct _string_array *sa);
void _sa_free (struct _string_array *sa);
bool _sa_add_copy (struct _string_array *sa, const char *s);
bool __attribute__((deprecated))  _sa_add (struct _string_array *sa, const char *s);
bool _sa_add_literal (struct _string_array *sa, const char *s);
bool _sa_add_ref (struct _string_array *sa, char *s);
char *_sa_get (struct _string_array *sa);

#endif
