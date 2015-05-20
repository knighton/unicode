from time import time

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
        nnn = map(lambda seg: map(lambda s: int(s, 16), seg.split()), ss)
        ss = map(lambda nn: ''.join(map(unichr, nn)), nnn)
        yield ss


def check(u, orig, nfc, nfd, nfkc, nfkd):
    t0 = time()
    assert u.normalize('NFC', orig) == nfc
    assert u.normalize('NFC', nfc) == nfc
    assert u.normalize('NFC', nfd) == nfc
    assert u.normalize('NFC', nfkc) == nfkc
    assert u.normalize('NFC', nfkd) == nfkc
    t1 = time()
    assert u.normalize('NFD', orig) == nfd
    assert u.normalize('NFD', nfc) == nfd
    assert u.normalize('NFD', nfd) == nfd
    assert u.normalize('NFD', nfkc) == nfkd
    assert u.normalize('NFD', nfkd) == nfkd
    t2 = time()
    assert u.normalize('NFKC', orig) == nfkc
    assert u.normalize('NFKC', nfc) == nfkc
    assert u.normalize('NFKC', nfd) == nfkc
    assert u.normalize('NFKC', nfkc) == nfkc
    assert u.normalize('NFKC', nfkd) == nfkc
    t3 = time()
    assert u.normalize('NFKD', orig) == nfkd
    assert u.normalize('NFKD', nfc) == nfkd
    assert u.normalize('NFKD', nfd) == nfkd
    assert u.normalize('NFKD', nfkc) == nfkd
    assert u.normalize('NFKD', nfkd) == nfkd
    t4 = time()
    return t1 - t0, t2 - t1, t3 - t2, t4 - t3


def dump_percentiles(name, tt):
    tt.sort()
    pcts = [0, 10, 50, 90, 99, 99.9]
    ss = ['times:']
    for pct in pcts:
        x = int(len(tt) * pct / 100.0)
        ss.append('%sth=%.3fus' % (
            str(pct).replace('.0', ''), tt[x] * 1000000))
    print '%s' % name, ' '.join(ss)


def test_normalization(name, mgr):
    ok = 0
    fail = 0
    nfc_tt = []
    nfd_tt = []
    nfkc_tt = []
    nfkd_tt = []
    for orig, nfc, nfd, nfkc, nfkd in each_test_line(TEST_F):
	try:
            nfc_t, nfd_t, nfkc_t, nfkd_t = \
                check(mgr, orig, nfc, nfd, nfkc, nfkd)
            nfc_tt.append(nfc_t)
            nfd_tt.append(nfd_t)
            nfkc_tt.append(nfkc_t)
            nfkd_tt.append(nfkd_t)
            ok += 1
        except:
            fail += 1

    assert ok + fail

    print '%s:' % name
    print '* ok: %d' % ok
    print '* fail: %d' % fail
    dump_percentiles('NFC', nfc_tt)
    dump_percentiles('NFD', nfd_tt)
    dump_percentiles('NFKC', nfkc_tt)
    dump_percentiles('NFKD', nfkd_tt)


def main():
    u = UnicodedataUnicodeManager()
    test_normalization('unicodedata', u)


if __name__ == '__main__':
    main()
