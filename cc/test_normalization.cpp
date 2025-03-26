#include <algorithm>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

#include "cc/files.h"
#include "cc/strings.h"
#include "cc/supertime.h"
#include "cc/unicode_manager.h"

using std::ifstream;
using std::string;

#define NFC_F "./data/nfc.txt"
#define NFKC_F "./data/nfkc.txt"
#define TEST_F "./data/NormalizationTest.txt"

namespace {

uint64_t NCheck(
        const UnicodeManager& mgr, const ustring& in,
        UnicodeNormalizationMethod unm, const ustring& expected_out) {
    ustring s = in;
    uint64_t t0 = supertime::NanosSinceEpoch();
    assert(mgr.Normalize(unm, &s));
    uint64_t t1 = supertime::NanosSinceEpoch();
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
    return t1 - t0;
}

void NormalizationEntry(
        const UnicodeManager& mgr, const vector<ustring>& forms,
        vector<uint64_t>* nfcs, vector<uint64_t>* nfds, vector<uint64_t>* nfkcs,
        vector<uint64_t>* nfkds) {
    assert(forms.size() == 5);

    auto& orig = forms[0];
    auto& nfc = forms[1];
    auto& nfd = forms[2];
    auto& nfkc = forms[3];
    auto& nfkd = forms[4];

    uint64_t r;

    r = NCheck(mgr, orig, UNM_NFC, nfc);
    nfcs->emplace_back(r);
    r = NCheck(mgr, nfc,  UNM_NFC, nfc);
    nfcs->emplace_back(r);
    r = NCheck(mgr, nfd,  UNM_NFC, nfc);
    nfcs->emplace_back(r);
    r = NCheck(mgr, nfkc, UNM_NFC, nfkc);
    nfcs->emplace_back(r);
    r = NCheck(mgr, nfkd, UNM_NFC, nfkc);
    nfcs->emplace_back(r);

    r = NCheck(mgr, orig, UNM_NFD, nfd);
    nfds->emplace_back(r);
    r = NCheck(mgr, nfc,  UNM_NFD, nfd);
    nfds->emplace_back(r);
    r = NCheck(mgr, nfd,  UNM_NFD, nfd);
    nfds->emplace_back(r);
    r = NCheck(mgr, nfkc, UNM_NFD, nfkd);
    nfds->emplace_back(r);
    r = NCheck(mgr, nfkd, UNM_NFD, nfkd);
    nfds->emplace_back(r);

    r = NCheck(mgr, orig, UNM_NFKC, nfkc);
    nfkcs->emplace_back(r);
    r = NCheck(mgr, nfc,  UNM_NFKC, nfkc);
    nfkcs->emplace_back(r);
    r = NCheck(mgr, nfd,  UNM_NFKC, nfkc);
    nfkcs->emplace_back(r);
    r = NCheck(mgr, nfkc, UNM_NFKC, nfkc);
    nfkcs->emplace_back(r);
    r = NCheck(mgr, nfkd, UNM_NFKC, nfkc);
    nfkcs->emplace_back(r);

    r = NCheck(mgr, orig, UNM_NFKD, nfkd);
    nfkds->emplace_back(r);
    r = NCheck(mgr, nfc,  UNM_NFKD, nfkd);
    nfkds->emplace_back(r);
    r = NCheck(mgr, nfd,  UNM_NFKD, nfkd);
    nfkds->emplace_back(r);
    r = NCheck(mgr, nfkc, UNM_NFKD, nfkd);
    nfkds->emplace_back(r);
    r = NCheck(mgr, nfkd, UNM_NFKD, nfkd);
    nfkds->emplace_back(r);
}

void DumpPercentiles(const string& name, const vector<uint64_t>& ff) {
    assert(ff.size());

    vector<size_t> percentiles = {0, 100, 500, 900, 990, 999};

    vector<size_t> indexes;
    for (auto& n : percentiles) {
        indexes.emplace_back(ff.size() * n / 1000);
    }

    printf("%s:\n", name.data());
    for (auto i = 0u; i < indexes.size(); ++i) {
        auto& pctile = percentiles[i];
        auto& index = indexes[i];
        printf("* %03lu/1000 %.3lfus\n", pctile,
               static_cast<double>(ff[index]) / 1000.0);
    }
    printf("\n");
}

void TestNormalization(const UnicodeManager& mgr, const string& norm_test_f) {
    ifstream in(norm_test_f);
    assert(in.good());

    string line;
    vector<string> code_list_strs;
    vector<string> code_strs;
    vector<ustring> forms;

    vector<uint64_t> nfcs;
    vector<uint64_t> nfds;
    vector<uint64_t> nfkcs;
    vector<uint64_t> nfkds;

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

        NormalizationEntry(mgr, forms, &nfcs, &nfds, &nfkcs, &nfkds);
    }

    sort(nfcs.begin(), nfcs.end());
    sort(nfds.begin(), nfds.end());
    sort(nfkcs.begin(), nfkcs.end());
    sort(nfkds.begin(), nfkds.end());
    printf("OK: %zu.  Fail: 0.\n", nfcs.size() / 5);  // asserts on failure.
    DumpPercentiles("NFC", nfcs);
    DumpPercentiles("NFD", nfds);
    DumpPercentiles("NFKC", nfkcs);
    DumpPercentiles("NFKD", nfkds);
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
