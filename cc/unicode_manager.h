#ifndef CC_UNICODE_MANAGER_H_
#define CC_UNICODE_MANAGER_H_

#include "cc/korean_manager.h"
#include "cc/unicode.h"
#include "cc/unicode_manager.h"

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

enum UnicodeNormalizationMethod {
    UNM_NFD,
    UNM_NFC,
    UNM_NFKD,
    UNM_NFKC,
};

template <typename Container>
struct ContainerHash {
    size_t operator()(const Container& c) const;
};

class UnicodeManager {
  public:
    // Init from fields.
    bool Init(const unordered_map<ucode, UnicodeCombiningClass>& c2k,
              const unordered_map<ucode, ustring>& nfd_c2cc,
              const unordered_map<ucode, ustring>& nfkd_c2cc,
              const unordered_map<ustring, ucode, ContainerHash<ustring>>&
                  compose_pair2c,
              const KoreanManager& korean, string* error);

    // Load from ICU's nfc.txt, nfkc.txt.
    bool InitFromFiles(const string& nfc_f, const string& nfkc_f,
                       string* error);

    // Get the canonical combining class of a character.
    UnicodeCombiningClass GetCombiningClass(ucode c) const;

    // Get each grapheme cluster (valid sequence of jamos, starter plus any
    // combining characters after it, etc.) in the string.  Example:
    //
    //     [     A   ring] [A + ring] [Angstrom]
    //     [U+0041 U+030A] [  U+00C5] [  U+212B]
    //
    // Returns true iff it got a span.
    bool EachUPC(const ustring& in, size_t* index, UnicodeSpan* span) const;

    // Normalize a Unicode string.
    //
    // Returns false on unknown normalization method.
    bool Normalize(UnicodeNormalizationMethod method, ustring* s) const;

  private:
    // Called by EachUPC().  Returns true iff it got a span.
    bool EachJamoUPC(const ustring& in, size_t* index, UnicodeSpan* span) const;

    // Called by EachUPC().  Returns true iff it got a span.
    bool EachNormalUPC(const ustring& in, size_t* index,
                       UnicodeSpan* span) const;

    // Correctly order the combining characters of a user-perceived character.
    //
    // * Each code point's canonical combining class must be in ascending order.
    // * For code points with the same canonical combining class, their original
    //   order is preserved (blocking characters affect composition).
    //
    // Returns whether it modified the string.
    bool ReorderUPC(const UnicodeSpan& span, ustring* inout) const;

    // Correctly order all combining characters in the text.
    //
    // Returns whether it modified the string.
    bool Reorder(ustring* inout) const;

    // Perform one iteration of decomposition.
    //
    // Returns whether it modified the string.
    bool DecomposeStep(const unordered_map<ucode, ustring>& decomp_c2cc,
                       ustring* inout) const;

    // Decompose and reorder the code points given a mapping.
    void Decompose(const unordered_map<ucode, ustring>& decomp_c2cc,
                   ustring* inout) const;

    // Returns whether it modified the string.
    bool IntraUPCComposeOnce(const ustring& in, const UnicodeSpan& span,
                             ustring* out) const;

    // Returns whether it modified the string.
    bool IntraUPCComposeStep(ustring* inout) const;

    // Returns whether it modified the string.
    bool InterUPCComposeStep(ustring* inout) const;

    // Decompose, reorder, and recompose the code points given mappings.
    void Compose(const unordered_map<ucode, ustring>& decomp_c2cc,
                 ustring* inout) const;

    unordered_map<ucode, UnicodeCombiningClass> c2k_;
    ucode first_nonzero_k_ucode_;
    ucode last_nonzero_k_ucode_;

    unordered_map<ucode, ustring> nfd_c2cc_;
    unordered_map<ucode, ustring> nfkd_c2cc_;
    unordered_map<ustring, ucode, ContainerHash<ustring>> compose_pair2c_;

    KoreanManager korean_;
};

#endif  // CC_UNICODE_MANAGER_H_
