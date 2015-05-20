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

bool UnicodeManager::ReorderUPC(
        const UnicodeSpan& span, ustring* inout) const {
    auto ok = true;
    for (auto i = span.begin + 1; i < span.end_excl; ++i) {
        auto& prev_c = (*inout)[i - 1];
        auto& c = (*inout)[i];
        auto prev_k = GetCombiningClass(prev_c);
        auto k = GetCombiningClass(c);
        if (k < prev_k) {
            ok = false;
            break;
        }
    }

    if (ok) {
        return false;
    }

    map<UnicodeCombiningClass, vector<ucode>> k2cc;
    for (auto i = span.begin; i < span.end_excl; ++i) {
        auto& c = (*inout)[i];
        auto k = GetCombiningClass(c);
        k2cc[k].emplace_back(c);
    }

    auto i = span.begin;
    for (auto it : k2cc) {
        for (auto& c : it.second) {
            (*inout)[i++] = c;
        }
    }

    return true;
}

bool UnicodeManager::Reorder(ustring* inout) const {
    size_t index = 0;
    UnicodeSpan span;
    bool changed = false;
    while (EachUPC(*inout, &index, &span)) {
        changed |= ReorderUPC(span, inout);
    }
    return changed;
}

bool UnicodeManager::DecomposeStep(
        const unordered_map<ucode, ustring>& decomp_c2cc,
        ustring* inout) const {
    bool changed = false;
    ustring rr;
    for (auto& c : *inout) {
        auto it = decomp_c2cc.find(c);
        if (it != decomp_c2cc.end()) {
            for (auto& r : it->second) {
                rr.emplace_back(r);
            }
            changed = true;
        } else {
            rr.emplace_back(c);
        }
    }

    changed |= Reorder(&rr);

    if (changed) {
        *inout = rr;
    }
    return changed;
}

void UnicodeManager::Decompose(
        const unordered_map<ucode, ustring>& decomp_c2cc,
        ustring* inout) const {
    korean_.Decompose(inout);
    while (DecomposeStep(decomp_c2cc, inout)) {
        ;
    }
}

bool UnicodeManager::IntraUPCComposeOnce(
        const ustring& in, const UnicodeSpan& span, ustring* out) const {
    // Look for matches.
    for (auto i = span.begin + 1; i < span.end_excl; ++i) {
        // If it's the same combining class as the previous code point, it's
        // blocked from combining.
        if (span.begin + 1 < i) {
            auto prev_k = GetCombiningClass(in[i - 1]);
            auto k = GetCombiningClass(in[i]);
            if (prev_k == k) {
                continue;
            }
        }

        // Look up the pair of (starter, non-starter) in the compositions.
        ustring pair;
        pair.emplace_back(in[span.begin]);
        pair.emplace_back(in[i]);
        auto it = compose_pair2c_.find(pair);
        if (it == compose_pair2c_.end()) {
            continue;
        }

        // We found it, so add its composition, then the others.  Then we're
        // done.
        auto& composed = it->second;
        out->emplace_back(composed);
        for (auto j = span.begin + 1; j < span.end_excl; ++j) {
            out->emplace_back(in[j]);
        }
        return true;
    }

    // Didn't find any matches.
    for (auto i = span.begin; i < span.end_excl; ++i) {
        out->emplace_back(in[i]);
    }
    return false;
}

bool UnicodeManager::IntraUPCComposeStep(ustring* inout) const {
    size_t index = 0;
    UnicodeSpan span;
    bool changed = false;
    auto& in = *inout;
    ustring out;
    while (EachUPC(in, &index, &span)) {
        changed |= IntraUPCComposeOnce(in, span, &out);
    }
    if (changed) {
        *inout = out;
    }
    return changed;
}

bool UnicodeManager::InterUPCComposeStep(ustring* inout) const {
    auto& in = *inout;
    auto i = 0u;
    auto changed = false;
    ustring out;
    while (i < in.size() - 1) {
        auto& first = in[i];
        auto& second = in[i + 1];
        ustring pair;
        pair.emplace_back(first);
        pair.emplace_back(second);
        auto it = compose_pair2c_.find(pair);
        if (it != compose_pair2c_.end()) {
            auto& composed = it->second;
            out.emplace_back(composed);
            i += 2;
            changed = true;
        } else {
            out.emplace_back(first);
            ++i;
        }
    }
    if (i == in.size() - 1) {
        out.emplace_back(in[i]);
    }
    if (changed) {
        *inout = out;
    }
    return changed;
}

void UnicodeManager::Compose(
        const unordered_map<ucode, ustring>& decomp_c2cc,
        ustring* inout) const {
    Decompose(decomp_c2cc, inout);

    korean_.Compose(inout);

    while (IntraUPCComposeStep(inout)) {
        ;
    }

    while (InterUPCComposeStep(inout)) {
        ;
    }
}

bool UnicodeManager::Normalize(
        UnicodeNormalizationMethod method, ustring* s) const {
    switch (method) {
    case UNM_NFD:
        Decompose(nfd_c2cc_, s);
        break;
    case UNM_NFC:
        Compose(nfd_c2cc_, s);
        break;
    case UNM_NFKD:
        Decompose(nfkd_c2cc_, s);
        break;
    case UNM_NFKC:
        Compose(nfkd_c2cc_, s);
        break;
    default:
        return false;
    }

    return true;
}
