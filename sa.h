#ifndef _SA_H_
#define _SA_H_
#include <stdbool.h>

struct _string_array {
	size_t num_strings;
	size_t total_len;
	char **strings;
};

bool _sa_init (struct _string_array *sa);
void _sa_free (struct _string_array *sa);
bool _sa_add (struct _string_array *sa, const char *s);
char *_sa_get (struct _string_array *sa);

#endif
