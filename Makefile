# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# Directories
INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data   # Put test PDFs here

# Source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/PDFParser.cpp $(SRC_DIR)/Analyzer.cpp

# Executable
TARGET = pdfanalyzer

# Default: build
all: $(TARGET)

# Build executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $(SRCS) -o $(TARGET)

# Run test
test: $(TARGET)
	@echo "Running PDF analyzer on test file..."
	./$(TARGET) $(DATA_DIR)/sample.pdf

# Clean build
clean:
	rm -f $(TARGET)
