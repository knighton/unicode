#ifndef CC_FILES_H_
#define CC_FILES_H_

#include <fstream>
#include <string>

using std::ifstream;
using std::string;

namespace files {

bool EachCommentableLine(ifstream* in, string* line);

}  // namespace files

#endif  // CC_FILES_H_
