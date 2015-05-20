#ifndef CC_STRINGS_H_
#define CC_STRINGS_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace strings {

// Split a string |s| into pieces separated by |c| into |v|.
//
// Examples:
//   ("Hello, World\n", ',') -> ["Hello", " World\n"]
//   ("", 'c') -> [""]
//   ("c", 'c') -> ["", ""]
//   ("cc", 'c') -> ["", "", ""]
void Split(const string& s, char c, vector<string>* v);

// Split by whitespace.  Ignore whitespace on either end.
void SplitByWhitespace(const string& s, vector<string>* v);

// Trim both sides.
void Trim(string* s);

// Equivalents of printf that work on strings, from gflags source code.
// Clears output before writing to it.
void SStringPrintf(string* output, const char* format, ...);
void StringAppendF(string* output, const char* format, ...);
string StringPrintf(const char* format, ...);

}  // namespace string

#endif  // CC_STRINGS_H_
