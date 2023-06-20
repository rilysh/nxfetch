#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "util.h"

#ifdef IEC_SPEC
    static char *suff[11] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "RB", "QB" };
    #define UNIT    1024.0
#else
    static char *suff[11] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB", "RiB", "QiB" };
    #define UNIT    1000.0
#endif

void pxerr(const char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

void *xcalloc(size_t lim)
{
    void *buf;
    
    buf = calloc(lim, sizeof(char));

    if (buf == NULL)
        pxerr("calloc()");

    return buf;
}

char *tofupper(char *src)
{
    int c;
    static char buf[MAX_VAL_SZ] = {0};

    buf[0] = (char)toupper(src[0]);

    /* To ignore first charecter in src */
    for (c = 0; c < 1; src++, c++);
    do {
        c = tolower(*src);
        strncat(buf, (char *)&c, 1L);
    } while (*src++);

    return buf;
}

char *toalllower(char *src)
{
    int c;
    size_t i;
    static char dst[MAX_VAL_SZ] = {0};

    i = 0;

    while (*src != '\0') {
        c = tolower(*src);
        memcpy(dst + i, (char *)&c, 1L);

        src++;
        i++;
    }

    return dst;
}

void human_bytes(char *dst, size_t size)
{
    int idx;
    double base, res;

    if (size > 0) {
        base = log10((double)size) / log10(UNIT);
        res = round(powf(UNIT, (float)(base - floor(base))) * 10.0) / 10.0;
        idx = (int)floor(base);
    }

    size <= 0 ? sprintf(dst, "0 B") :
        sprintf(dst, "%0.1lf %s", res, suff[idx]);
}

int strccnt(const char *str, char character)
{
    int occr;

    occr = 0;

    do {
        if (*str == character)
            occr++;
    } while (*str++);

    return occr;
}

char *strccut(char *src, char c)
{
    size_t len, tlen;

    len = tlen = strlen(src);

    while (len--) {
        if (strccnt(src, c) == 0)
            break;

        if (len == 1)
            len = tlen;

        src++;
    }

    return src;
}

char *strrep(
    const char *src, const char *needle,
    const char *put, int depth
)
{
    char *pos;
    size_t nd_len, pt_len, mn_size;

    pos = NULL;
    nd_len = strlen(needle);
    pt_len = strlen(put);

    while (depth--) {
        pos = strstr(src, needle);

        if (pos == NULL)
            return NULL;

        mn_size = strlen(pos) + nd_len + 1;

        /* If one of them is larger than other */
        memmove(pos + pt_len, pos + nd_len, mn_size);
        memcpy(pos, put, pt_len);
    }

    return pos;
}

char *word_trim(char *src, size_t max_len)
{
    size_t sz;

    sz = strlen(src);

    if (sz > max_len) {
        do {
            src[sz - 1] = '\0';
            sz--;
        } while (sz != max_len);
    }

    return src;
}

char *read_value(const char *src, const char *key)
{
    char *str;
    size_t sz, i;
    static char buf[MAX_VAL_SZ] = {0};

    str = strstr(src, key);
    
    if (str == NULL)
        return NULL;

    sz = strlen(key) + 1;
    i = 0;

    /* Ignore first key and the equal sign */
    for (; sz; sz--, str++);

    while (*str != '\n') {
        memcpy(buf + i, (char *)&*str, 1);
        str++;
        i++;
    }

    return buf;
}
