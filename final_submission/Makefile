CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Isrc
FLEX     := flex

BUILD    := build
SRC      := src
TARGET   := $(BUILD)/lexer

.PHONY: all clean test

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/lex.yy.cpp: $(SRC)/lexer.l $(SRC)/token_types.h | $(BUILD)
	$(FLEX) -o $(BUILD)/lex.yy.cpp $(SRC)/lexer.l

$(TARGET): $(BUILD)/lex.yy.cpp
	$(CXX) $(CXXFLAGS) $(BUILD)/lex.yy.cpp -o $(TARGET)

test: $(TARGET)
	bash run.sh

clean:
	rm -rf $(BUILD) test-results
