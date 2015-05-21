#include "unicode.h"

#include <cstdio>

void DumpUString(const ustring& s) {
    printf("[");
    if (s.size()) {
        printf("%x", s[0]);
    }
    for (auto i = 1u; i < s.size(); ++i) {
        printf(" %x", s[i]);
    }
    printf("]\n");
}
