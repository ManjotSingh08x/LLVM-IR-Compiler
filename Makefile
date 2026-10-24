UNAME    := $(shell uname -s)

# Homebrew keeps llvm off the PATH, so llvm-config is looked up there on macOS.
# both may be overridden: make CXX=g++-14 LLVM_CONFIG=/usr/bin/llvm-config-20
ifeq ($(UNAME),Darwin)
LLVM_CONFIG ?= $(shell brew --prefix llvm 2>/dev/null)/bin/llvm-config
# homebrew llvm is built against libc++, so its headers are compiled with clang
CXX         ?= clang++
else
LLVM_CONFIG ?= llvm-config
CXX         ?= g++
endif

LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS     := $(shell $(LLVM_CONFIG) --libs support) \
                 $(shell $(LLVM_CONFIG) --system-libs)

# llvm-config reports its own warning and standard settings, ours come after it
CXXFLAGS := $(LLVM_CXXFLAGS) -std=c++17 -Wall -Isrc -Ibuild
LDFLAGS  := $(LLVM_LDFLAGS)
LDLIBS   := $(LLVM_LIBS)

FLEX     := flex
BISON    := bison

BUILD    := build
SRC      := src
TARGET   := $(BUILD)/parser
PARSER   := $(BUILD)/parser.tab.cpp
PSTAMP   := $(BUILD)/parser.stamp
SCANNER  := $(BUILD)/lex.yy.cpp
AST_SRCS := $(wildcard $(SRC)/ast/*.cpp) $(wildcard $(SRC)/semantic/*.cpp)
AST_HDRS := $(wildcard $(SRC)/ast/*.h) $(wildcard $(SRC)/semantic/*.h) $(SRC)/declarator.h

.PHONY: all clean test

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(PSTAMP): $(SRC)/parser.y | $(BUILD)
	$(BISON) -d -v -b $(BUILD)/parser $(SRC)/parser.y
	mv $(BUILD)/parser.tab.c $(PARSER)
	touch $(PSTAMP)

$(SCANNER): $(SRC)/lexer.l $(SRC)/token_types.h $(PSTAMP) | $(BUILD)
	$(FLEX) -o $(SCANNER) $(SRC)/lexer.l

$(TARGET): $(PSTAMP) $(SCANNER) $(AST_SRCS) $(AST_HDRS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSER) $(SCANNER) $(AST_SRCS) $(LDLIBS) -o $(TARGET)

# run.sh builds first, so this target does not depend on $(TARGET)
test:
	@bash run.sh

clean:
	rm -rf $(BUILD) outputs
