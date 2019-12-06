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
		if (sa->strings[i].d)
			sa->strings[i].d(sa->strings[i].s);
	}
	free(sa->strings);
	sa->slots_used = 0;
	sa->total_len = 0;
	sa->slots_available = 0;
	sa->strings = NULL;
}

bool _sa_add_aux (struct _string_array *sa, char *s, void (*d)(void *))
{
	if (!s)
		return true;
	
	if (sa->strings == NULL) {
		sa->slots_available = _sa_next(0);
		sa->slots_used = 0;
		sa->total_len = 0;
		sa->strings = malloc(
			sa->slots_available * sizeof(struct _sa_string_s));
	} else if (sa->slots_used == sa->slots_available) {
		size_t new_slots_available = _sa_next(sa->slots_available);
		struct _sa_string_s *temp = realloc(sa->strings,
			new_slots_available * sizeof(struct _sa_string_s));
		if (temp == NULL) return false;
		sa->slots_available = new_slots_available;
		sa->strings = temp;
	}

	sa->slots_used++;
	sa->strings[sa->slots_used-1].s = s;
	sa->strings[sa->slots_used-1].d = d;
	sa->total_len = sa->total_len + strlen(s);
	return true;
}

bool _sa_add_ref (struct _string_array *sa, char *s)
{
	return _sa_add_aux(sa, s, free);
}

bool _sa_add_literal (struct _string_array *sa, const char *s)
{
	return _sa_add_aux(sa, (char *)s, NULL);
}

bool _sa_add_copy (struct _string_array *sa, const char *s)
{
	if (!s) return false;
	char *newstring = strdup(s);
	return _sa_add_aux(sa, newstring, free);
}

char *_sa_get (struct _string_array *sa)
{
	char *t = malloc(sa->total_len + 1);
	char *p = t;
	for (size_t i = 0; i < sa->slots_used; i++) {
		size_t len = strlen(sa->strings[i].s);
		memcpy(p, sa->strings[i].s, len);
		p += len;
	}
	p[0] = '\0';
	return t;
}
