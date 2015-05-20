#include "files.h"

#include "cc/strings.h"

namespace files {

bool EachCommentableLine(ifstream* in, string* line) {
    while (getline(*in, *line)) {
        size_t x = line->find('#');
        if (x != ~0ul) {
            line->resize(x);
        }
        strings::Trim(line);
        if (strings::IsSpace(*line)) {
            continue;
        }
        return true;
    }
    return false;
}

}  // namespace files
