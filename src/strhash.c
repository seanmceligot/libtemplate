#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libtemplate/strhash.h"
#include "libtemplate/mybool.h"
#include "libtemplate/debug.h"
#include "libtemplate/xmalloc.h"
/*
 	* a really simple non-optimized key,value list
	* maybe I will improve it someday.
*/
static int g_verbose_fail = TRUE;
static int g_fail = 0;

StrHash *
strhash_new (int count, int grow)
{
  StrHash *hash;
  
	assert (count > 0, "strhash initial size must be > 0");

  hash = xmalloc (sizeof (StrHash));
  hash->allocated = count;
  hash->hash =
    (StrHashKeyValue *) xmalloc (hash->allocated * sizeof (StrHashKeyValue));
  hash->count = 0;
  hash->grow = grow;
  return hash;
}

void
strhash_put (StrHash * hash, char *key, void *data)
{
  hash->count++;
  if (hash->count > hash->allocated) {
    hash->allocated += hash->grow;
    hash->hash = realloc (hash->hash, hash->allocated * sizeof (StrHashKeyValue));
  }
  hash->hash[hash->count - 1].key = key;
  hash->hash[hash->count - 1].data = data;
}

char *
strhash_find (StrHash * hash, const char *key)
{
  int i;
  for (i = 0; i < hash->count; i++) {
    if (strcmp (hash->hash[i].key, key) == 0) {
      return hash->hash[i].data;
    }
  }
  return NULL;

}
void
strhash_debugf (StrHash * hash)
{
  int i;
  for (i = 0; i < hash->count; i++) {
    fprintf (stdout, "key (%s) ", hash->hash[i].key);
    fprintf (stdout, "val (%s) \n", (char *) hash->hash[i].data);
  }
}
void
strhash_free (StrHash * hash)
{
	if (hash) {
	  xfree (hash->hash);
		xfree(hash);
	}
}

void
strhash_foreach (StrHash * hash, StrHashForEachFunc userfunc, void *user_data)
{
	int i;
	if (!hash) {
					return;
	}
  for (i = 0; i < hash->count; i++) {
			userfunc(hash->hash[i].key, hash->hash[i].data, user_data);
	}
}

void
strhash_free_each (StrHash * hash, StrHashForEachFunc userfunc, void *user_data)
{
	int i;

	if (!hash) {
					return;
	}

  for (i = 0; i < hash->count; i++) {
			userfunc(hash->hash[i].key, hash->hash[i].data, user_data);
	}
	strhash_free(hash);
}
