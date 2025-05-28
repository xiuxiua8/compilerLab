# Makefile for Compiler Project

CXX = g++
CXXFLAGS = -std=c++14 -Wall -g
LDFLAGS =

FILE = ./code/16.src

# Target files
TARGETS = dfa lexer lr0 semantic_analyzer intermediate_code_generator 

# Default target
all: $(TARGETS)

# Lexical Analyzer
dfa: lab1/dfa.cpp
	$(CXX) $(CXXFLAGS) -DDFA_MAIN -o $@ $< $(LDFLAGS)

lexer: lexer.cpp
	$(CXX) $(CXXFLAGS) -DLEXER_MAIN -o $@ $< $(LDFLAGS)
	
# Syntax Analyzer
lr0: lab3/lr0.cpp
	$(CXX) $(CXXFLAGS) -DLR0_MAIN -o $@ $< $(LDFLAGS)

# Semantic Analyzer 
semantic_analyzer: semantic_analyzer.cpp
	$(CXX) $(CXXFLAGS) -DSEMANTIC_ANALYZER_MAIN -o $@ $< $(LDFLAGS)

# Intermediate Code Generator
intermediate_code_generator: intermediate_code_generator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Clean
clean:
	rm -rf $(TARGETS) *.o 

# Full test procedure
test: all
	@echo "=== Testing Lexical Analyzer ==="
	./lexer $(FILE)
	@echo "\n=== Testing Syntax Analyzer ==="
	./lr0
	@echo "\n=== Testing Semantic Analyzer ==="
	./semantic_analyzer --debug
	@echo "\n=== Testing Compiler Driver ==="
	./intermediate_code_generator $(FILE)
	
.PHONY: all clean test