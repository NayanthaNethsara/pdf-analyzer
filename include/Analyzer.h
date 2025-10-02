#pragma once
#include "PDFParser.h"

struct PDFBreakdown
{
    size_t totalSize = 0;
    size_t imagesSize = 0;
    size_t fontsSize = 0;
    size_t metadataSize = 0;

    size_t textSize = 0;       // total text size
    size_t realTextSize = 0;   // BT ... ET streams
    size_t vectorTextSize = 0; // vectorized glyphs
    size_t otherTextSize = 0;  // leftover text objects
};

class Analyzer
{
private:
    const PDFParser &parser;
    PDFBreakdown breakdown;

public:
    Analyzer(const PDFParser &p);
    void computeBreakdown();
    void printSummary() const;
    void printFileSize() const;
    const PDFBreakdown &getBreakdown() const;
};
