#ifndef __STRHASH_H
#define __STRHASH_H

typedef void (*StrHashForEachFunc) (void* key, void *data, void *user_data);

typedef struct _StrHashKeyValue StrHashKeyValue;
struct _StrHashKeyValue {
  char *key;
  void *data;
};

typedef struct _StrHash StrHash;
struct _StrHash {
  StrHashKeyValue *hash;
  int count;
  int allocated;
  int grow;
};
StrHash *strhash_new (int count, int grow);
void strhash_put (StrHash * hash, char *key, void *data);
char *strhash_find (StrHash * hash, const char *key);
void strhash_debugf (StrHash * hash);
void strhash_free (StrHash * hash);
void strhash_foreach (StrHash * hash, StrHashForEachFunc userfunc, void *user_data);
void strhash_free_each (StrHash * hash, StrHashForEachFunc userfunc, void *user_data);

#endif
