.PHONY: all build run clean

HPP_FILES := $(wildcard ../*.hpp)
CPP_FILES := $(wildcard ./*.cpp)
ALL_DEPENDENCIES := $(HPP_FILES) $(CPP_FILES) Makefile

BIN_DIR := bin

BIN = $(BIN_DIR)/test.bin

CXX := g++
CXXFLAGS := \
	-std=c++23 \
	-I../../ \
	$$(libsixel-config --cflags) \
	-DSHAPOBROT_ENABLE_ASSERT \
	-g
LDFLAGS := \
	-lm \
	-lstdc++_libbacktrace \
	$$(libsixel-config --libs)

all: build

build: $(BIN)

$(BIN): $(ALL_DEPENDENCIES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(CPP_FILES) -o $@ $(LDFLAGS)

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BIN_DIR)

