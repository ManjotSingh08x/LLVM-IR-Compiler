UNAME    := $(shell uname -s)

# llvm-config is looked up in Homebrew on macOS, override with make CXX=... LLVM_CONFIG=...
ifeq ($(UNAME),Darwin)
LLVM_CONFIG ?= $(shell brew --prefix llvm 2>/dev/null)/bin/llvm-config
# homebrew llvm is built against libc++, so its headers are compiled with clang
CXX         ?= clang++
else
LLVM_CONFIG ?= llvm-config
CXX         ?= g++
endif

CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags) -std=c++17 -Wall -Isrc -Ibuild
LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS   := $(shell $(LLVM_CONFIG) --libs support) $(shell $(LLVM_CONFIG) --system-libs)

BUILD    := build
TARGET   := $(BUILD)/parser
PARSER   := $(BUILD)/parser.tab.cpp
PSTAMP   := $(BUILD)/parser.stamp
SCANNER  := $(BUILD)/lex.yy.cpp

MODULES  := ast semantic ir dataflow passes pipeline codegen
SRCS     := $(foreach m,$(MODULES),$(wildcard src/$(m)/*.cpp))
HDRS     := $(foreach m,$(MODULES),$(wildcard src/$(m)/*.h)) src/declarator.h

.PHONY: all test clean diagram

all: $(TARGET)

$(PSTAMP): src/parser.y
	mkdir -p $(BUILD)
	bison -d -v -b $(BUILD)/parser src/parser.y
	mv $(BUILD)/parser.tab.c $(PARSER)
	touch $(PSTAMP)

$(SCANNER): src/lexer.l src/token_types.h $(PSTAMP)
	flex -o $(SCANNER) src/lexer.l

$(TARGET): $(PSTAMP) $(SCANNER) $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSER) $(SCANNER) $(SRCS) $(LDLIBS) -o $(TARGET)

test: $(TARGET)
	@bash run_tests.sh

clean:
	rm -rf $(BUILD) outputs

# README architecture diagram: needs tectonic and pdftocairo (poppler)
diagram: docs/architecture.svg

docs/architecture.svg: docs/architecture.tex
	cd docs && tectonic architecture.tex
	pdftocairo -svg docs/architecture.pdf docs/architecture.svg
	rm -f docs/architecture.pdf
