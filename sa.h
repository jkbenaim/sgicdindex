#ifndef _SA_H_
#define _SA_H_
#include <stdbool.h>

struct _sa_string_t {
	char *string;
	void (*dtor)(char *);
};

struct _string_array {
	size_t slots_used;
	size_t total_len;
	size_t slots_available;
	char **strings;
	//struct _sa_string_t *strings;
};

size_t _sa_next(size_t old) __attribute__((pure));
bool _sa_init (struct _string_array *sa);
void _sa_free (struct _string_array *sa);
bool _sa_add (struct _string_array *sa, const char *s) __attribute__((hot));
char *_sa_get (struct _string_array *sa);

#endif
