# Makefile for Compiler Project

CXX = g++
CXXFLAGS = -std=c++14 -Wall -g
LDFLAGS =

# Target files
TARGETS = semantic_analyzer compiler_driver intermediate_code_generator 

# Default target
all: $(TARGETS)

# Semantic Analyzer (for independent testing)
semantic_analyzer: semantic_analyzer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Compiler Driver
compiler_driver: compiler_driver.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Intermediate Code Generator
intermediate_code_generator: intermediate_code_generator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)


# Generate SLR table for Experiment Four
lab3/slr_output.txt: lab3/lr0
	cd lab3 && ./lr0 > slr_output.txt

# Test Compiler Driver
test-driver: compiler_driver lab1/dfa
	./compiler_driver code/10.src

# Test Intermediate Code Generator
test-icg: intermediate_code_generator
	./intermediate_code_generator

# Test Integrated Compiler
test: integrated_compiler
	./integrated_compiler code/10.src

# Test Semantic Analyzer
test-semantic: semantic_analyzer
	./semantic_analyzer --debug

# Test Lexical Analyzer
test-lexer: lab1/dfa
	cd lab1 && ./dfa

# Clean
clean:
	rm -f $(TARGETS) *.o slr_table_init.cpp lab3/slr_output.txt tokens.txt tokens.tmp

# Full test procedure
test-all: all
	@echo "=== Testing Lexical Analyzer ==="
	cd lab1 && echo "3" | ./dfa ../code/10.src
	@echo "\n=== Testing Syntax Analyzer ==="
	cd lab3 && ./lr0
	@echo "\n=== Testing Semantic Analyzer ==="
	./semantic_analyzer --debug
	@echo "\n=== Testing Compiler Driver ==="
	./compiler_driver code/10.src

.PHONY: all clean test test-semantic test-lexer test-driver test-all