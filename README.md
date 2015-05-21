#### Overview

This is a minimalist Unicode library that does normalization and user-perceived character segmentation.  Two identical versions, in C++ and pure python.

#### age:

    $ make all
    $ make test_cc
    $ make test_py
    
### Correctness:

Everything passes against [unicode.org](http://unicode.org)'s [NormalizationTest.txt](http://unicode.org/Public/UNIDATA/NormalizationTest.txt) for Unicode 7.0.0.

Python's unicodedata module uses 5.2.0.
    
#### Performance:

From [Macchiato FAQ](http://www.macchiato.com/unicode/nfc-faq): "according to data at Google ... ~99.98% of web HTML page content characters are definitely NFC".  Combining characters are rare in practice.

NormalizationTest.txt contains all manner of combining characters and should be expected to skew performance toward the worst case.

Performance against NormalizationTest.txt on my machine:

C++ (microsec)

|      | 0th   | 10th  | 50th  | 90th  | 99th  | 99.9th |
| ---- | ----: | ----: | ----: | ----: | ----: | -----: |
| NFC  | 0.217 | 0.256 | 0.567 | 1.408 | 2.920 | 3.887  |
| NFD  | 0.098 | 0.128 | 0.420 | 0.558 | 1.408 | 1.734  |
| NFKC | 0.218 | 0.318 | 0.599 | 1.535 | 2.870 | 3.775  |
| NFKD | 0.098 | 0.192 | 0.433 | 0.585 | 1.462 | 1.764  |

Python unicodedata module (microsec)

501 normalizations wrong, 18080 correct against Unicode 7.0.0 data.

|      | 0th   | 10th  | 50th  | 90th  | 99th  | 99.9th |
| :--- | ----: | ----: | ----: | ----: | ----: | -----: |
| NFC  | 0.954 | 0.954 | 1.907 | 3.099 | 5.960 | 10.014 |
| NFD  | 0.954 | 0.954 | 1.907 | 3.099 | 5.007 |  6.199 |
| NFKC | 0.954 | 0.954 | 1.907 | 4.053 | 6.199 | 11.921 |
| NFKD | 0.954 | 0.954 | 1.907 | 3.099 | 5.007 |  6.914 |

Pure python (microsec)

|      | 0th    | 10th   | 50th   | 90th    | 99th    | 99.9th  |
| :--- | -----: | -----: | -----: | ------: | ------: | ------: |
| NFC  | 39.816 | 41.962 | 69.141 | 101.805 | 177.860 | 200.987 |
| NFD  | 20.981 | 21.935 | 42.915 |  53.167 |  75.102 |  95.844 |
| NFKC | 42.915 | 49.829 | 67.949 | 115.871 | 174.999 | 221.014 |
| NFKD | 23.842 | 30.994 | 43.869 |  59.843 |  76.056 | 121.117 |
