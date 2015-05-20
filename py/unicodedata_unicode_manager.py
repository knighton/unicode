import unicodedata

from unicode_manager import UnicodeManager


class UnicodedataUnicodeManager(UnicodeManager):
    """
    Internally relies on the unicodedata python extension.
    """

    def each_upc(self, nn):
        cccs = map(unicodedata.combining, nn)

        begins = [0]
        for i in range(1, len(nn)):
            ccc = cccs[i]
            if not ccc:
                begins.append(i)
        begins.append(len(nn))

        for i in range(len(begins) - 1):
            begin = begins[i]
            end_excl = begins[i + 1]
            yield nn[begin : end_excl]

    def normalize(self, method, s):
        return unicodedata.normalize(method, s)
