#include "korean_manager.h"

#define UNUSED(a) (a + 0)

KoreanManager::KoreanManager() :
        hangul_base_(0xAC00), hangul_end_incl_(0xD7A3),
        jamo_initial_base_(0x1100), jamo_medial_base_(0x1161),
        jamo_final_base_(0x11A7), jamo_end_incl_(0x11FF),
        jamo_initial_count_(19), jamo_medial_count_(21),
        jamo_final_count_(28) {
    UNUSED(jamo_initial_count_);
}

bool KoreanManager::IsHangul(ucode c) const {
    return hangul_base_ <= c && c <= hangul_end_incl_;
}

bool KoreanManager::IsJamo(ucode c) const {
    return jamo_initial_base_ <= c && c <= jamo_end_incl_;
}

bool KoreanManager::IsInitialJamo(ucode c) const {
    return jamo_initial_base_ <= c && c < jamo_medial_base_;
}

bool KoreanManager::IsMedialJamo(ucode c) const {
    return jamo_medial_base_ <= c && c < jamo_final_base_;
}

bool KoreanManager::IsFinalJamo(ucode c) const {
    return jamo_final_base_ <= c && c <= jamo_final_base_;
}

ucode KoreanManager::HangulFromTwoJamo(ucode a, ucode b) const {
    a -= jamo_initial_base_;
    b -= jamo_medial_base_;
    a *= jamo_medial_count_ * jamo_final_count_;
    b *= jamo_final_count_;
    return hangul_base_ + a + b;
}

ucode KoreanManager::HangulFromThreeJamo(ucode a, ucode b, ucode c) const {
    a -= jamo_initial_base_;
    b -= jamo_medial_base_;
    c -= jamo_final_base_;
    a *= jamo_medial_count_ * jamo_final_count_;
    b *= jamo_final_count_;
    return hangul_base_ + a + b + c;
}

bool KoreanManager::AppendJamoFromHangul(ucode hangul, ustring* out) const {
    if (!IsHangul(hangul)) {
        return false;
    }

    hangul -= hangul_base_;
    auto c = hangul % jamo_final_count_;
    hangul /= jamo_final_count_;
    auto b = hangul % jamo_medial_count_;
    auto a = hangul / jamo_medial_count_;

    a += jamo_initial_base_;
    out->emplace_back(a);
    b += jamo_medial_base_;
    out->emplace_back(b);
    if (c) {
        c += jamo_final_base_;
        out->emplace_back(c);
    }
    return true;
}

void KoreanManager::Compose(ustring* inout) const {
    size_t i = 0;
    auto& in = *inout;
    ustring out;
    while (i + 2 < in.size()) {
        if (!IsInitialJamo(in[i])) {
            out.emplace_back(in[i]);
            ++i;
            continue;
        }

        if (!IsMedialJamo(in[i + 1])) {
            out.emplace_back(in[i]);
            out.emplace_back(in[i + 1]);
            i += 2;
            continue;
        }

        if (IsFinalJamo(in[i + 2])) {
            auto r = HangulFromThreeJamo(in[i], in[i + 1], in[i + 2]);
            out.emplace_back(r);
            i += 3;
        } else {
            auto r = HangulFromTwoJamo(in[i], in[i + 1]);
            out.emplace_back(r);
            i += 2;
        }
    }

    if (2 <= in.size() && i == in.size() - 2 &&
            IsInitialJamo(in[i]) && IsMedialJamo(in[i + 1])) {
        auto r = HangulFromTwoJamo(in[i], in[i + 1]);
        out.emplace_back(r);
    } else {
        while (i < in.size()) {
            out.emplace_back(in[i]);
            ++i;
        }
    }

    in = out;
}

void KoreanManager::Decompose(ustring* inout) const {
    ustring out;
    for (auto& c : *inout) {
        if (!AppendJamoFromHangul(c, &out)) {
            out.emplace_back(c);
        }
    }
    *inout = out;
}
