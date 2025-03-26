from collections import defaultdict

from korean_normalizer import KoreanNormalizer
from unicode_manager import UnicodeManager
from util import each_line


def load_nfkc(code2ccc, nfkc_f, nfkd_c2cc):
    for line in each_line(nfkc_f):
        a, b = line.split('>')
        from_c = int(a, 16)
        to_cc = list(map(lambda s: int(s, 16), b.split()))
        nfkd_c2cc[from_c] = to_cc


def load_nfc_combining_class(line, code2ccc):
    a, b = line.split(':')
    ccc = int(b)
    if '..' in a:
        from_s, to_s = a.split('..')
    else:
        from_s, to_s = a, a
    for n in range(int(from_s, 16), int(to_s, 16) + 1):
        code2ccc[n] = ccc


def load_nfc_nfd_only(code2ccc, line, nfd_c2cc):
    a, b = line.split('>')
    from_c = int(a, 16)
    to_cc = list(map(lambda s: int(s, 16), b.split()))
    kkk = list(map(lambda code: code2ccc.get(code, 0), to_cc))
    assert kkk == sorted(kkk)
    nfd_c2cc[from_c] = to_cc


def load_nfc_bidirectional(code2ccc, line, nfc_cc2c, nfd_c2cc):
    a, b = line.split('=')
    from_c = int(a, 16)
    to_cc = list(map(lambda s: int(s, 16), b.split()))
    kkk = list(map(lambda code: code2ccc.get(code, 0), to_cc))
    assert kkk == sorted(kkk)
    assert len(to_cc) == 2
    nfc_cc2c[tuple(to_cc)] = from_c
    nfd_c2cc[from_c] = to_cc


def load_nfc(nfc_f, code2ccc, nfc_cc2c, nfd_c2cc):
    for line in each_line(nfc_f):
        if ':' in line:
            load_nfc_combining_class(line, code2ccc)
        elif '>' in line:
            load_nfc_nfd_only(code2ccc, line, nfd_c2cc)
        elif '=' in line:
            load_nfc_bidirectional(code2ccc, line, nfc_cc2c, nfd_c2cc)


class PythonUnicodeManager(object):
    """
    Pure python implementation of Unicode normalization and segmentation.
    """

    def __init__(self, code2ccc, compose_pair2c, nfd_c2cc, nfkd_c2cc):
        # Code point -> canonical combining class.
        self.code2ccc = code2ccc  # code -> int.  0 if key dne.

        # Canonical/compatibility normalizations.
        self.compose_pair2c = compose_pair2c  # code pair -> code.
        self.nfd_c2cc = nfd_c2cc  # code -> list of codes.
        self.nfkd_c2cc = nfkd_c2cc  # code -> list of codes.

        # Korean normalization is algorithmic.
        self.korean = KoreanNormalizer()

    @staticmethod
    def from_files(nfc_f, nfkc_f):
        code2ccc = {}
        compose_pair2c = {}
        nfd_c2cc = {}
        load_nfc(nfc_f, code2ccc, compose_pair2c, nfd_c2cc)

        nfkd_c2cc = dict(nfd_c2cc)
        load_nfkc(code2ccc, nfkc_f, nfkd_c2cc)

        return PythonUnicodeManager(
            code2ccc, compose_pair2c, nfd_c2cc, nfkd_c2cc)

    def _order_upc(self, nn, cccs):
        ccc2nn = defaultdict(list)
        for n, ccc in zip(nn, cccs):
            ccc2nn[ccc].append(n)
        rr = []
        for ccc in sorted(ccc2nn):
            rr += ccc2nn[ccc]
        return rr

    def each_upc(self, nn):
        cccs = []
        for n in nn:
            ccc = self.code2ccc.get(n, 0)
            cccs.append(ccc)

        begins = [0]
        for i in range(1, len(nn)):
            ccc = cccs[i]
            if not ccc:
                begins.append(i)
        begins.append(len(nn))

        for i in range(len(begins) - 1):
            begin = begins[i]
            end_excl = begins[i + 1]
            sub_nn = nn[begin : end_excl]
            sub_cccs = cccs[begin : end_excl]
            yield self._order_upc(sub_nn, sub_cccs)

    def order_combining_characters(self, nn):
        rr = []
        for sub_nn in self.each_upc(nn):
            rr += sub_nn
        return rr

    def decompose_step(self, nn, code2codes):
        rr2 = []
        for n in nn:
            rr2 += code2codes.get(n, [n])
        return self.order_combining_characters(rr2)

    def decompose(self, nn, code2codes):
        prev_rr = self.korean.decompose(nn)
        while True:
            rr = self.decompose_step(prev_rr, code2codes)
            if rr == prev_rr:
                break
            prev_rr = rr
        return rr

    def intra_grapheme_cluster_compose_step(self, nn):
        rr = []
        for sub_nn in self.each_upc(nn):
            found = False
            for i in range(1, len(sub_nn)):
                if 2 <= i:
                    # If it's the same combining class as prev char, it's
                    # blocked from combining.
                    prev_ccc = self.code2ccc.get(sub_nn[i - 1], 0)
                    this_ccc = self.code2ccc.get(sub_nn[i], 0)
                    if prev_ccc == this_ccc:
                        continue

                pair = (sub_nn[0], sub_nn[i])
                r = self.compose_pair2c.get(pair, None)
                if r is not None:
                    rr.append(r)
                    for j in range(1, len(sub_nn)):
                        if j != i:
                            rr.append(sub_nn[j])
                    found = True
                    break
            if not found:
                rr += sub_nn
        return rr

    def inter_grapheme_cluster_compose_step(self, nn):
        rr = []
        i = 0
        while i < len(nn) - 1:
            first = nn[i]
            second = nn[i + 1]
            pair = (first, second)
            composed = self.compose_pair2c.get(pair, None)
            if composed is None:
                rr.append(first)
                i += 1
            else:
                rr.append(composed)
                i += 2
        if i == len(nn) - 1:
            rr.append(nn[i])
        else:
            assert i == len(nn)
        return rr

    def compose(self, nn, code2codes):
        prev_rr = self.decompose(nn, code2codes)

        prev_rr = self.korean.compose(prev_rr)

        while True:
            rr = self.intra_grapheme_cluster_compose_step(prev_rr)
            if rr == prev_rr:
                break
            prev_rr = rr

        while True:
            rr = self.inter_grapheme_cluster_compose_step(prev_rr)
            if rr == prev_rr:
                break
            prev_rr = rr

        return rr

    def normalize(self, method, s):
        nn = map(ord, s)
        if method == 'NFC':
            nn = self.compose(nn, self.nfd_c2cc)
        elif method == 'NFD':
            nn = self.decompose(nn, self.nfd_c2cc)
        elif method == 'NFKC':
            nn = self.compose(nn, self.nfkd_c2cc)
        elif method == 'NFKD':
            nn = self.decompose(nn, self.nfkd_c2cc)
        else:
            assert False

        s = ''.join(map(chr, nn))
        return s
