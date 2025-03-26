from time import time_ns

from python_unicode_manager import PythonUnicodeManager
from unicodedata_unicode_manager import UnicodedataUnicodeManager
from util import each_line


NFC_F = '../data/nfc.txt'
NFKC_F = '../data/nfkc.txt'
TEST_F = '../data/NormalizationTest.txt'


def each_test_line(f):
    for line in each_line(f):
        if line.startswith('@'):
            continue
        ss = line.split(';')[:-1]
        nnn = list(map(lambda seg: list(map(lambda s: int(s, 16), seg.split())), ss))
        ss = list(map(lambda nn: ''.join(map(chr, nn)), nnn))
        yield ss


def check(u, orig, nfc, nfd, nfkc, nfkd):
    t0 = time_ns()
    nfc0 = u.normalize('NFC', orig) == nfc
    nfc1 = u.normalize('NFC', nfc) == nfc
    nfc2 = u.normalize('NFC', nfd) == nfc
    nfc3 = u.normalize('NFC', nfkc) == nfkc
    nfc4 = u.normalize('NFC', nfkd) == nfkc
    t1 = time_ns()
    nfd0 = u.normalize('NFD', orig) == nfd
    nfd1 = u.normalize('NFD', nfc) == nfd
    nfd2 = u.normalize('NFD', nfd) == nfd
    nfd3 = u.normalize('NFD', nfkc) == nfkd
    nfd4 = u.normalize('NFD', nfkd) == nfkd
    t2 = time_ns()
    nfkc0 = u.normalize('NFKC', orig) == nfkc
    nfkc1 = u.normalize('NFKC', nfc) == nfkc
    nfkc2 = u.normalize('NFKC', nfd) == nfkc
    nfkc3 = u.normalize('NFKC', nfkc) == nfkc
    nfkc4 = u.normalize('NFKC', nfkd) == nfkc
    t3 = time_ns()
    nfkd0 = u.normalize('NFKD', orig) == nfkd
    nfkd1 = u.normalize('NFKD', nfc) == nfkd
    nfkd2 = u.normalize('NFKD', nfd) == nfkd
    nfkd3 = u.normalize('NFKD', nfkc) == nfkd
    nfkd4 = u.normalize('NFKD', nfkd) == nfkd
    t4 = time_ns()

    nfc = all([nfc0, nfc1, nfc2, nfc3, nfc4])
    nfd = all([nfd0, nfd1, nfd2, nfd3, nfd4])
    nfkc = all([nfkc0, nfkc1, nfkc2, nfkc3, nfkc4])
    nfkd = all([nfkd0, nfkd1, nfkd2, nfkd3, nfkd4])
    ok = all([nfc, nfd, nfkc, nfkd])

    return ok, t1 - t0, t2 - t1, t3 - t2, t4 - t3


def dump_percentiles(name, tt):
    assert tt
    tt.sort()
    print('%s times:' % name)
    for ile in [0, 100, 500, 900, 990, 1000]:
        idx = min(len(tt) * ile // 1000, len(tt) - 1)
        print('%3d/1000: %6dns' % (min(ile, 1000 - 1), tt[idx]))
    print()


def test_normalization(name, mgr):
    ok = 0
    fail = 0
    nfc_tt = []
    nfd_tt = []
    nfkc_tt = []
    nfkd_tt = []
    for orig, nfc, nfd, nfkc, nfkd in each_test_line(TEST_F):
        is_ok, nfc_t, nfd_t, nfkc_t, nfkd_t = \
            check(mgr, orig, nfc, nfd, nfkc, nfkd)
        if is_ok:
            ok += 1
        else:
            fail += 1
        nfc_tt.append(nfc_t)
        nfd_tt.append(nfd_t)
        nfkc_tt.append(nfkc_t)
        nfkd_tt.append(nfkd_t)

    assert ok + fail

    print('%s:' % name)
    print('* ok: %d' % ok)
    print('* fail: %d' % fail)
    dump_percentiles('NFC', nfc_tt)
    dump_percentiles('NFD', nfd_tt)
    dump_percentiles('NFKC', nfkc_tt)
    dump_percentiles('NFKD', nfkd_tt)
    print


def main():
    u = UnicodedataUnicodeManager()
    test_normalization('unicodedata', u)

    u = PythonUnicodeManager.from_files(NFC_F, NFKC_F)
    test_normalization('pure python', u)


if __name__ == '__main__':
    main()
