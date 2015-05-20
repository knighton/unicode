class UnicodeManager(object):
    def each_upc(self, nn):
        """
        list of code points -> yields list of code points

        Yield each user-perceived character in the string, also known as
        grapheme cluster.  These are lists of code points that begin with a
        starter (a code point whose canonical combining class is 0) or Jamo
        doubles or triples.
        """
        raise NotImplementedError

    def normalize(self, method, nn):
        """
        method, list of code points -> list of normalized code points

        Does NFC/NFD/NFKC/NFKD normalization on the text.
        """
        raise NotImplementedError
