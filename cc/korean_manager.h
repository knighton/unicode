#ifndef CC_KOREAN_MANAGER_H_
#define CC_KOREAN_MANAGER_H_

#include "cc/unicode.h"

#include <string>

using std::string;

class KoreanManager {
  public:
    // Set up constants.
    KoreanManager();

    // Range checks.
    bool IsHangul(ucode c) const;
    bool IsJamo(ucode c) const;
    bool IsInitialJamo(ucode c) const;
    bool IsMedialJamo(ucode c) const;
    bool IsFinalJamo(ucode c) const;

    // Hangul -> Jamo.
    //
    // Returns false if input is not a valid Hangul code point.
    bool AppendJamoFromHangul(ucode hangul, ustring* out) const;

    // Jamo -> Hangul.
    //
    // Assumes inputs are valid Jamo code points.
    ucode HangulFromTwoJamo(ucode a, ucode b) const;
    ucode HangulFromThreeJamo(ucode a, ucode b, ucode c) const; 

    // Replace Jamo -> Hangul in text.
    void Compose(ustring* inout) const;

    // Replace Hangul -> Jamo in text.
    void Decompose(ustring* inout) const;

  private:
    ucode hangul_base_;
    ucode hangul_end_incl_;

    ucode jamo_initial_base_;
    ucode jamo_medial_base_;
    ucode jamo_final_base_;
    ucode jamo_end_incl_;

    ucode jamo_initial_count_;
    ucode jamo_medial_count_;
    ucode jamo_final_count_;
};

#endif  // CC_KOREAN_MANAGER_H_
