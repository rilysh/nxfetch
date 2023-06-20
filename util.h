#ifndef UTIL_H
#define UTIL_H      1

#include <stddef.h>

#define MAX_VAL_SZ      80

char *tofupper(char *src);
void human_bytes(char *dst, size_t size);
void pxerr(const char *err);
void *xcalloc(size_t lim);
char *toalllower(char *src);
int strccnt(const char *str, char character);
char *strccut(char *src, char c);
char *strrep(const char *src, const char *needle, const char *put, int depth);
char *word_trim(char *src, size_t max_len);
char *read_value(const char *src, const char *key);

#endif /** UTIL_H */
