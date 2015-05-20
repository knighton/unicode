#include <cstdio>
#include <string>

#include "cc/unicode_manager.h"

using std::string;

#define NFC_F "./data/nfc.txt"
#define NFKC_F "./data/nfkc.txt"

int main() {
    UnicodeManager u;
    string error;
    if (!u.InitFromFiles(NFC_F, NFKC_F, &error)) {
        fprintf(stderr, "Init failed: %s\n", error.data());
    }
}
