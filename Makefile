CC = clang++

SRC_ROOT = ./
BIN_DIR = bin/

FLAGS_BASE = \
	-std=c++11 \
	-fcolor-diagnostics \
	-O3 \
	-ferror-limit=5 \
	-I$(SRC_ROOT) \

FLAGS_WARN = \
	-Wpedantic \
	-Wall \
	-Weverything \
	-Wextra \
	-Werror \

FLAGS_WARN_DISABLE = \
	-Wno-c++98-compat-pedantic \
	-Wno-covered-switch-default \
	-Wno-exit-time-destructors \
	-Wno-global-constructors \
	-Wno-padded \
	-Wno-weak-vtables \

FLAGS = $(FLAGS_BASE) $(FLAGS_WARN) $(FLAGS_WARN_DISABLE)

all:
	mkdir -p $(BIN_DIR)
	$(CC) `find -type f -name "*.cc"` $(SRC_ROOT)/cc/test_normalization.cpp -o $(BIN_DIR)/test_normalization $(FLAGS)

test_cc:
	./bin/test_normalization

test_py:
	cd py; python3 test_normalization.py

clean:
	rm -rf $(BIN_DIR)

