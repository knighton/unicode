#ifndef CC_UNICODE_H_
#define CC_UNICODE_H_

#include <cstddef>
#include <cstdint>
#include <vector>

using std::vector;

// A single Unicode code point.  Not to be confused with a user-perceived
// character which may contain multiple code points (use UnicodeSpan).
typedef uint32_t ucode;

// A unicode code point's canonical combining class (see
// http://www.unicode.org/versions/Unicode7.0.0/ch03.pdf#G49537).
typedef uint8_t UnicodeCombiningClass;

// A string containing Unicode code points.
typedef vector<ucode> ustring;

// Dump the Unicode string's hex values to stdout.
void DumpUString(const ustring& s);

// A user-perceived Unicode character.
//
// Contains indexes into a ustring.  Does not own memory.
struct UnicodeSpan {
    size_t begin;
    size_t end_excl;
};

#endif  // CC_UNICODE_H_
