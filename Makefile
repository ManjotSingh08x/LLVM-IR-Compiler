CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Isrc -Ibuild
FLEX     := flex
BISON    := bison

BUILD    := build
SRC      := src
TARGET   := $(BUILD)/parser
PARSER   := $(BUILD)/parser.tab.cpp
PSTAMP   := $(BUILD)/parser.stamp
SCANNER  := $(BUILD)/lex.yy.cpp
AST_SRCS := $(wildcard $(SRC)/ast/*.cpp)
AST_HDRS := $(wildcard $(SRC)/ast/*.h)

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
	$(CXX) $(CXXFLAGS) $(PARSER) $(SCANNER) $(AST_SRCS) -o $(TARGET)

# run.sh builds first, so this target does not depend on $(TARGET)
test:
	@bash run.sh

clean:
	rm -rf $(BUILD) outputs
