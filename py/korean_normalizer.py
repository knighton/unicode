class KoreanNormalizer(object):
    def __init__(self):
        # Hangul range.
        self.hangul_base = 0xAC00
        self.hangul_end_incl = 0xD7A3

        # Jamo ranges.
        self.initial_base = 0x1100
        self.medial_base = 0x1161
        self.final_base = 0x11A7
        self.jamo_end_incl = 0x11FF

        self.initial_count = 19
        self.medial_count = 21
        self.final_count = 28

    def is_hangul(self, n):
        return self.hangul_base <= n <= self.hangul_end_incl
    
    def is_jamo(self, n):
        return self.initial_base <= n <= self.jamo_end_incl

    def is_initial_jamo(self, n):
        return self.initial_base <= n < self.medial_base

    def is_medial_jamo(self, n):
        return self.medial_base <= n < self.final_base

    def is_final_jamo(self, n):
        return self.final_base <= n <= self.jamo_end_incl

    def hangul_from_two_jamo(self, a, b):
        a -= self.initial_base
        b -= self.medial_base
        a *= self.medial_count * self.final_count
        b *= self.final_count
        return self.hangul_base + a + b

    def hangul_from_three_jamo(self, a, b, c):
        a -= self.initial_base
        b -= self.medial_base
        c -= self.final_base
        a = a * self.medial_count * self.final_count
        b = b * self.final_count
        return self.hangul_base + a + b + c

    def jamo_from_hangul(self, n):
        n -= self.hangul_base
        final = n % self.final_count
        n /= self.final_count
        medial = n % self.medial_count
        n /= self.medial_count
        initial = n
        initial += self.initial_base
        medial += self.medial_base
        if final:
            final += self.final_base
            return [initial, medial, final]
        else:
            return [initial, medial]

    def compose(self, nn):
        rr = []
        i = 0
        while i < len(nn) - 2:
            if not self.is_initial_jamo(nn[i]):
                rr.append(nn[i])
                i += 1
                continue

            if not self.is_medial_jamo(nn[i + 1]):
                rr.append(nn[i])
                rr.append(nn[i + 1])
                i += 2
                continue

            if self.is_final_jamo(nn[i + 2]):
                r = self.hangul_from_three_jamo(nn[i], nn[i + 1], nn[i + 2])
                rr.append(r)
                i += 3
            else:
                r = self.hangul_from_two_jamo(nn[i], nn[i + 1])
                rr.append(r)
                i += 2

        if 2 <= len(nn) and i == len(nn) - 2 and \
                self.is_initial_jamo(nn[i]) and self.is_medial_jamo(nn[i + 1]):
            r = self.hangul_from_two_jamo(nn[i], nn[i + 1])
            rr.append(r)
        else:
            while i < len(nn):
                rr.append(nn[i])
                i += 1
        return rr

    def decompose(self, nn):
        rr = []
        for n in nn:
            if self.is_hangul(n):
                rr += self.jamo_from_hangul(n)
            else:
                rr.append(n)
        return rr
