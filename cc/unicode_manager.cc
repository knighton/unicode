#include "unicode_manager.h"

#include <boost/functional/hash.hpp>
#include <map>
#include <fstream>

using std::getline;
using std::ifstream;
using std::map;

template <typename Container>
size_t ContainerHash<Container>::operator()(const Container& c) const {
    return boost::hash_range(c.begin(), c.end());
}

bool UnicodeManager::Init(
        const unordered_map<ucode, UnicodeCombiningClass>& c2k,
        const unordered_map<ucode, ustring>& nfd_c2cc,
        const unordered_map<ucode, ustring>& nfkd_c2cc,
        const unordered_map<ustring, ucode, ContainerHash<ustring>>&
            compose_pair2c,
        const KoreanManager& korean, string* error) {
}

bool UnicodeManager::InitFromFiles(
        const string& nfc_f, const string& nfkc_f, string* error) {
}

UnicodeCombiningClass UnicodeManager::GetCombiningClass(ucode c) const {
}

bool UnicodeManager::EachUPC(
        const ustring& in, size_t* x, UnicodeSpan* span) const {
}

bool UnicodeManager::Normalize(
        UnicodeNormalizationMethod method, ustring* s) const {
}
