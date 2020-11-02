
#include "utils.h"
#include <string.h>

char * nstrdup(const char * str) {
    char * rv;
    if (!str) return NULL;
    rv = new char[strlen(str) + 1];
    strcpy(rv, str);
    return rv;
}
