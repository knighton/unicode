#include "unicode_manager.h"

#include <boost/functional/hash.hpp>
#include <map>
#include <fstream>

#include "cc/strings.h"

using std::getline;
using std::ifstream;
using std::map;

template <typename Container>
size_t ContainerHash<Container>::operator()(const Container& c) const {
    return boost::hash_range(c.begin(), c.end());
}

namespace {

bool GetBounds(const unordered_map<ucode, UnicodeCombiningClass>& c2k,
               ucode* first_nonzero_k_ucode, ucode* last_nonzero_k_ucode) {
    if (c2k.empty()) {
        return false;
    }

    *first_nonzero_k_ucode = 0x10FFFF;
    *last_nonzero_k_ucode = 0;
    for (auto it : c2k) {
        auto& c = it.first;
        if (c < *first_nonzero_k_ucode) {
            *first_nonzero_k_ucode = c;
        }
        if (*last_nonzero_k_ucode < c) {
            *last_nonzero_k_ucode = c;
        }
    }

    return true;
}

bool ParseHexCodePoint(const string& in, ucode* out, string* error) {
    if (in.empty()) {
        *error = "[UnicodeManager] Empty code point string.";
        return false;
    }

    ucode r = 0;
    for (auto& c : in) {
        if ('0' <= c && c <= '9') {
            r *= 16;
            r += static_cast<ucode>(c - '0');
        } else if ('A' <= c && c <= 'F') {
            r *= 16;
            r += static_cast<ucode>(c - 'A');
        } else {
            *error = "[UnicodeManager] Invalid character in code point string.";
            return false;
        }
    }

    *out = r;
    return true;
}

bool ParseUInt8(const string& in, uint8_t* out, string* error) {
    if (in.empty()) {
        *error = "[UnicodeManager] Empty uint8_t string.";
        return false;
    }

    uint8_t r = 0;
    for (auto& c : in) {
        if ('0' <= c && c <= '9') {
            r *= 10;
            r += static_cast<ucode>(c - '0');
        } else {
            *error = "[UnicodeManager] Invalid character in uint8_t string.";
            return false;
        }
    }

    *out = r;
    return true;
}

bool HandleNFCLineCombiningClass(
        const string& line, size_t split,
        unordered_map<ucode, UnicodeCombiningClass>* c2k, string* error) {
    size_t dot = line.find('.');
    ucode begin;
    ucode end_incl;
    string s;
    if (dot == ~0ul) {
        s = line.substr(0, split);
        if (!ParseHexCodePoint(s, &begin, error)) {
            return false;
        }
        end_incl = begin;
    } else {
        if (!(dot + 1 < split && line[dot + 1] == '.')) {
            return false;
        }
        s = line.substr(0, dot);
        if (!ParseHexCodePoint(s, &begin, error)) {
            return false;
        }
        s = line.substr(dot + 2, split - (dot + 2));
        if (!ParseHexCodePoint(s, &end_incl, error)) {
            return false;
        }
    }
    uint8_t combining_class;
    s = line.substr(split + 1);
    if (!ParseUInt8(s, &combining_class, error)) {
        return false;
    }
    for (auto c = begin; c <= end_incl; ++c) {
        (*c2k)[c] = combining_class;
    }
    return true;
}

bool SplitKeyToValues(const string& in, size_t split, ucode* from_c,
                      ustring* to_cc, string* error) {
    to_cc->clear();
    string s = in.substr(0, split);
    if (!ParseHexCodePoint(s, from_c, error)) {
        return false;
    }
    s = in.substr(split + 1);
    vector<string> ss;
    strings::Split(s, ' ', &ss);
    to_cc->reserve(ss.size());
    for (auto& item : ss) {
        ucode to_c;
        if (!ParseHexCodePoint(item, &to_c, error)) {
            return false;
        }
        to_cc->emplace_back(to_c);
    }
    return true;
}

bool HandleNFCLineNFDOnly(
        const string& line, size_t split,
        unordered_map<ucode, ustring>* nfd_c2cc, string* error) {
    ucode from_c;
    ustring to_cc;
    if (!SplitKeyToValues(line, split, &from_c, &to_cc, error)) {
        return false;
    }

    (*nfd_c2cc)[from_c] = to_cc;
    return true;
}

bool HandleNFCLineBidirectional(
        const string& line, size_t split,
        unordered_map<ucode, ustring>* nfd_c2cc,
        unordered_map<ustring, ucode, ContainerHash<ustring>>* compose_pair2c,
        string* error) {
    ucode from_c;
    ustring to_cc;
    if (!SplitKeyToValues(line, split, &from_c, &to_cc, error)) {
        return false;
    }

    (*nfd_c2cc)[from_c] = to_cc;
    (*compose_pair2c)[to_cc] = from_c;
    return true;
}

bool LoadNFCFile(
        const string& nfc_f, unordered_map<ucode, UnicodeCombiningClass>* c2k,
        unordered_map<ucode, ustring>* nfd_c2cc,
        unordered_map<ustring, ucode, ContainerHash<ustring>>* compose_pair2c,
        string* error) {
    ifstream in(nfc_f);
    string line;
    while (getline(in, line)) {
        size_t x = line.find(':');
        if (x != ~0ul) {
            if (HandleNFCLineCombiningClass(line, x, c2k, error)) {
                continue;
            } else {
                return false;
            }
        }

        x = line.find('>');
        if (x != ~0ul) {
            if (HandleNFCLineNFDOnly(line, x, nfd_c2cc, error)) {
                continue;
            } else {
                return false;
            }
        }

        x = line.find('=');
        if (x != ~0ul) {
            if (HandleNFCLineBidirectional(
                    line, x, nfd_c2cc, compose_pair2c, error)) {
                continue;
            } else {
                return false;
            }
        }

        *error = "[UnicodeManager] Invalid NFC config line.";
        return false;
    }

    return true;
}

bool LoadNFKCFile(const string& nfkc_f,
                  unordered_map<ucode, ustring>* nfkd_c2cc, string* error) {
    ifstream in(nfkc_f);
    string line;
    while (getline(in, line)) {
        size_t x = line.find('>');
        if (x == ~0ul) {
            *error = "[UnicodeManager] '>' missing from NFKC file line.";
            return false;
        }

        ucode from_c;
        ustring to_cc;
        if (!SplitKeyToValues(line, x, &from_c, &to_cc, error)) {
            return false;
        }

        (*nfkd_c2cc)[from_c] = to_cc;
    }

    return true;
}

}  // namespace

bool UnicodeManager::Init(
        const unordered_map<ucode, UnicodeCombiningClass>& c2k,
        const unordered_map<ucode, ustring>& nfd_c2cc,
        const unordered_map<ucode, ustring>& nfkd_c2cc,
        const unordered_map<ustring, ucode, ContainerHash<ustring>>&
            compose_pair2c,
        const KoreanManager& korean, string* error) {
    if (!GetBounds(c2k, &first_nonzero_k_ucode_, &last_nonzero_k_ucode_)) {
        *error = "[UnicodeManager] code point -> combining class mapping is "
                 "empty.";
        return false;
    }

    c2k_ = c2k_;
    compose_pair2c_ = compose_pair2c;
    nfd_c2cc_ = nfd_c2cc;
    nfkd_c2cc_ = nfkd_c2cc;
    korean_ = korean;
    return true;
}

bool UnicodeManager::InitFromFiles(
        const string& nfc_f, const string& nfkc_f, string* error) {
    unordered_map<ucode, UnicodeCombiningClass> c2k;
    unordered_map<ucode, ustring> nfd_c2cc;
    unordered_map<ustring, ucode, ContainerHash<ustring>> compose_pair2c;
    if (!LoadNFCFile(nfc_f, &c2k, &nfd_c2cc, &compose_pair2c, error)) {
        return false;
    }

    unordered_map<ucode, ustring> nfkd_c2cc;
    if (!LoadNFKCFile(nfkc_f, &nfkd_c2cc, error)) {
        return false;
    }

    KoreanManager korean;
    return Init(c2k, nfd_c2cc, nfkd_c2cc, compose_pair2c, korean, error);
}

UnicodeCombiningClass UnicodeManager::GetCombiningClass(ucode c) const {
    if (c < first_nonzero_k_ucode_) {
        return 0;
    }

    if (last_nonzero_k_ucode_ < c) {
        return 0;
    }

    auto it = c2k_.find(c);
    if (it != c2k_.end()) {
        return it->second;
    }

    return 0;
}

bool UnicodeManager::EachJamoUPC(
        const ustring& in, size_t* x, UnicodeSpan* span) const {
    span->begin = *x;
    ++(*x);
    if (*x == in.size() || !korean_.IsMedialJamo(in[*x])) {
        span->end_excl = *x;
        return true;
    }

    ++(*x);
    if (korean_.IsFinalJamo(in[*x])) {
        span->end_excl = *x + 1;
    } else {
        span->end_excl = *x;
    }
    return true;
}

bool UnicodeManager::EachNormalUPC(
        const ustring& in, size_t* x, UnicodeSpan* span) const {
    span->begin = *x;
    ++(*x);
    while (*x < in.size() && GetCombiningClass(in[*x])) {
        ++(*x);
    }
    span->end_excl = *x;
    return true;
}

bool UnicodeManager::EachUPC(
        const ustring& in, size_t* x, UnicodeSpan* span) const {
    if (!(*x < in.size())) {
        return false;
    }

    if (korean_.IsInitialJamo(in[*x])) {
        return EachJamoUPC(in, x, span);
    } else {
        return EachNormalUPC(in, x, span);
    }
}

bool UnicodeManager::Normalize(
        UnicodeNormalizationMethod method, ustring* s) const {
}
