#include <stdlib.h>
#include <string.h>
#include "sa.h"


#include <stdio.h>


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
	if (!s)
		return true;
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
	printf("_sa_get: %d strings, %d bytes\n", sa->num_strings, sa->total_len+1);
	for (size_t i = 0; i < sa->num_strings; i++) {
		size_t len = strlen(sa->strings[i]);
		memcpy(p, sa->strings[i], len);
		p += len;
	}
	p[0] = '\0';
	return t;
}
