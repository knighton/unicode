#include "strings.h"

namespace strings {

void Split(const string& s, char c, vector<string>* v) {
    v->clear();
    size_t prev_c = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == c) {
            v->emplace_back(s.substr(prev_c, i - prev_c));
            prev_c = i + 1;
        }
    }
    v->emplace_back(s.substr(prev_c, s.size() - prev_c));
}

#define INDEX_DNE ~0ul

void SplitByWhitespace(const string& s, vector<string>* v) {
    v->clear();

    size_t last_nonspace_i = INDEX_DNE;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (isspace(c)) {
            if (last_nonspace_i != INDEX_DNE) {
                v->emplace_back(s.substr(last_nonspace_i, i - last_nonspace_i));
                last_nonspace_i = INDEX_DNE;
            }
        } else {
            if (last_nonspace_i == INDEX_DNE) {
                last_nonspace_i = i;
            }
        }
    }
    if (last_nonspace_i != INDEX_DNE) {
        v->emplace_back(s.substr(last_nonspace_i));
    }
}

#undef INDEX_DNE

void Trim(string* s) {
    if (s->empty()) {
        return;
    }

    auto begin = ~0ul;
    for (auto i = 0ul; i < s->size(); ++i) {
        auto& c = (*s)[i];
        if (!isspace(c)) {
            begin = i;
            break;
        }
    }

    if (begin == ~0ul) {
        s->clear();
        return;
    }

    auto end = ~0ul;
    for (auto i = s->size() - 1; i != ~0ul; --i) {
        auto& c = (*s)[i];
        if (isspace(c)) {
            end = i;
            break;
        }
    }

    *s = s->substr(begin, end - begin);
}

static void InternalStringPrintf(string* output, const char* format, va_list ap) {
    char space[128];    // try a small buffer and hope it fits

    // It's possible for methods that use a va_list to invalidate
    // the data in it upon use.  The fix is to make a copy
    // of the structure before using it and use that copy instead.
    va_list backup_ap;
    va_copy(backup_ap, ap);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
    size_t bytes_written = static_cast<size_t>(vsnprintf(
        space, sizeof(space), format, backup_ap));
#pragma clang diagnostic pop
    va_end(backup_ap);

    if (bytes_written < sizeof(space)) {
        output->append(space, bytes_written);
        return;
    }

    // Repeatedly increase buffer size until it fits.
    size_t length = sizeof(space);
    while (true) {
        // We need exactly "bytes_written+1" characters
        length = bytes_written + 1;
        char* buf = new char[length];

        // Restore the va_list before we use it again
        va_copy(backup_ap, ap);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
        bytes_written = static_cast<size_t>(vsnprintf(
            buf, length, format, backup_ap));
#pragma clang diagnostic pop
        va_end(backup_ap);

        if (bytes_written < length) {
            output->append(buf, bytes_written);
            delete [] buf;
            return;
        }
        delete [] buf;
    }
}

void SStringPrintf(string* output, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    output->clear();
    InternalStringPrintf(output, format, ap);
    va_end(ap);
}

void StringAppendF(string* output, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    InternalStringPrintf(output, format, ap);
    va_end(ap);
}

string StringPrintf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    string output;
    InternalStringPrintf(&output, format, ap);
    va_end(ap);
    return output;
}

}  // namespace strings
