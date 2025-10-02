# PDF Analyzer

A simple C++ command-line tool to analyze PDF file sizes by component.  
It scans a PDF and reports the size of Images, Fonts, Metadata, and Text.

## Features

- Reads PDF as a binary file
- Detects PDF objects (`/XObject`, `/Font`, `/Metadata`, `/Contents`)
- Computes actual byte size per component
- Prints a clear size breakdown

## Requirements

- C++17 compatible compiler (GCC, Clang, or MSVC)

## Build

**On Mac / Linux**

```sh
g++ -std=c++17 main.cpp -o pdfanalyzer
```

**On Windows (MSVC)**

```sh
cl /EHsc main.cpp /Fe:pdfanalyzer.exe
```

## Usage

```sh
./pdfanalyzer <file.pdf>
```

## Example

```sh
./pdfanalyzer sample.pdf
```

### Sample Output

```
PDF Size Analysis:
Total Size: 3645722 bytes
Images: 1635096 bytes
Fonts: 94779 bytes
Metadata: 0 bytes
Text: 1915847 bytes
```
