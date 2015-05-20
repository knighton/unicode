#ifndef CC_STRINGS_IMPL_H_
#define CC_STRINGS_IMPL_H_

namespace strings {

template <typename int_t>
bool ParseDec(const string& s, int_t* n) {
    *n = 0;
    for (auto& c : s) {
        if ('0' <= c && c <= '9') {
            *n *= 10;
            *n += static_cast<int_t>(c - '0');
        } else {
            return false;
        }
    }
    return true;
}

template <typename int_t>
bool ParseHex(const string& s, int_t* n) {
    *n = 0;
    for (auto& c : s) {
        int_t digit;
        if ('0' <= c && c <= '9') {
            digit = static_cast<int_t>(c - '0');
        } else if ('A' <= c && c <= 'F') {
            digit = static_cast<int_t>(c - 'A');
        } else if ('a' <= c && c <= 'f') {
            digit = static_cast<int_t>(c - 'a');
        } else {
            return false;
        }
        *n *= 16;
        *n += digit;
    }
    return true;
}

}  // namespace

#endif  // CC_STRINGS_IMPL_H_
