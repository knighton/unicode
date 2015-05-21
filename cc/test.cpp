#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

#include "cc/files.h"
#include "cc/strings.h"
#include "cc/unicode_manager.h"

using std::ifstream;
using std::string;

#define NFC_F "./data/nfc.txt"
#define NFKC_F "./data/nfkc.txt"
#define TEST_F "./data/NormalizationTest.txt"

namespace {

void NCheck(
        const UnicodeManager& mgr, const ustring& in,
        UnicodeNormalizationMethod unm, const ustring& expected_out) {
    ustring s = in;
    assert(mgr.Normalize(unm, &s));
    if (s != expected_out) {
        printf("input: ");
        DumpUString(in);
        printf("normalization method: %d\n", unm);
        printf("expected output: ");
        DumpUString(expected_out);
        printf("actual output: ");
        DumpUString(s);
        assert(false);
    }
}

void NormalizationEntry(
        const UnicodeManager& mgr, const vector<ustring>& forms) {
    assert(forms.size() == 5);

    auto& orig = forms[0];
    auto& nfc = forms[1];
    auto& nfd = forms[2];
    auto& nfkc = forms[3];
    auto& nfkd = forms[4];

    NCheck(mgr, orig, UNM_NFC, nfc);
    NCheck(mgr, nfc,  UNM_NFC, nfc);
    NCheck(mgr, nfd,  UNM_NFC, nfc);
    NCheck(mgr, nfkc, UNM_NFC, nfkc);
    NCheck(mgr, nfkd, UNM_NFC, nfkc);

    NCheck(mgr, orig, UNM_NFD, nfd);
    NCheck(mgr, nfc,  UNM_NFD, nfd);
    NCheck(mgr, nfd,  UNM_NFD, nfd);
    NCheck(mgr, nfkc, UNM_NFD, nfkd);
    NCheck(mgr, nfkd, UNM_NFD, nfkd);

    NCheck(mgr, orig, UNM_NFKC, nfkc);
    NCheck(mgr, nfc,  UNM_NFKC, nfkc);
    NCheck(mgr, nfd,  UNM_NFKC, nfkc);
    NCheck(mgr, nfkc, UNM_NFKC, nfkc);
    NCheck(mgr, nfkd, UNM_NFKC, nfkc);

    NCheck(mgr, orig, UNM_NFKD, nfkd);
    NCheck(mgr, nfc,  UNM_NFKD, nfkd);
    NCheck(mgr, nfd,  UNM_NFKD, nfkd);
    NCheck(mgr, nfkc, UNM_NFKD, nfkd);
    NCheck(mgr, nfkd, UNM_NFKD, nfkd);
}

void TestNormalization(const UnicodeManager& mgr, const string& norm_test_f) {
    ifstream in(norm_test_f);
    assert(in.good());

    string line;
    vector<string> code_list_strs;
    vector<string> code_strs;
    vector<ustring> forms;
    while (files::EachCommentableLine(&in, &line)) {
        if (line[0] == '@') {
            continue;
        }

        strings::Split(line, ';', &code_list_strs);
        assert(code_list_strs.size() == 6);
        code_list_strs.pop_back();
        forms.clear();
        for (auto i = 0u; i < code_list_strs.size(); ++i) {
            strings::SplitByWhitespace(code_list_strs[i], &code_strs);
            ustring form;
            for (auto& s : code_strs) {
                ucode c;
                assert(strings::ParseHex(s, &c));
                form.emplace_back(c);
            }
            forms.emplace_back(form);
        }
        NormalizationEntry(mgr, forms);
    }
}

}  // namespace

int main() {
    UnicodeManager mgr;
    string error;
    if (!mgr.InitFromFiles(NFC_F, NFKC_F, &error)) {
        fprintf(stderr, "Init failed: %s\n", error.data());
        assert(false);
    }

    TestNormalization(mgr, TEST_F);
}
