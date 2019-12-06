#include <stdlib.h>
#include <string.h>
#include "sa.h"

size_t _sa_next(size_t old) {
	return old + 100;
}

bool _sa_init (struct _string_array *sa)
{
	sa->slots_used = 0;
	sa->total_len = 0;
	sa->slots_available = 0;
	sa->strings = NULL;
	return true;
}

void _sa_free (struct _string_array *sa)
{
	for (size_t i = 0; i < sa->slots_used; i++) {
		free(sa->strings[i]);
	}
	free(sa->strings);
	sa->slots_used = 0;
	sa->total_len = 0;
	sa->slots_available = 0;
	sa->strings = NULL;
}

bool _sa_add (struct _string_array *sa, const char *s)
{
	if (!s)
		return true;
	
	if (sa->strings == NULL) {
		sa->slots_available = _sa_next(0);
		sa->slots_used = 0;
		sa->total_len = 0;
		sa->strings = calloc(sa->slots_available, sizeof(char *));
	} else if (sa->slots_used == sa->slots_available) {
		size_t new_slots_available = _sa_next(sa->slots_available);
		char **temp = realloc(sa->strings,
			new_slots_available * sizeof(char *));
		if (temp == NULL) return false;
		sa->slots_available = new_slots_available;
		sa->strings = temp;
	}

	char *newstring = strdup(s);
	sa->strings[sa->slots_used++] = newstring;
	sa->total_len = sa->total_len + strlen(newstring);
	return true;
}

char *_sa_get (struct _string_array *sa)
{
	char *t = malloc(sa->total_len + 1);
	char *p = t;
	for (size_t i = 0; i < sa->slots_used; i++) {
		size_t len = strlen(sa->strings[i]);
		memcpy(p, sa->strings[i], len);
		p += len;
	}
	p[0] = '\0';
	return t;
}
