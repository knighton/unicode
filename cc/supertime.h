#ifndef CC_SUPERTIME_H_
#define CC_SUPERTIME_H_

#include <cstdint>
#include <string>

using std::string;

namespace supertime {

void PrettyTime(string* s);
uint64_t NanosSinceEpoch();

}  // namespace supertime

#endif  // CC_SUPERTIME_H_
