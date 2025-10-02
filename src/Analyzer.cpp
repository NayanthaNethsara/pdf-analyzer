#include "Analyzer.h"
#include <iostream>

Analyzer::Analyzer(const PDFParser &p) : parser(p) {}

void Analyzer::computeBreakdown()
{
    breakdown.totalSize = parser.getFileSize();
    breakdown.imagesSize = 0;
    breakdown.fontsSize = 0;
    breakdown.metadataSize = 0;
    breakdown.textSize = 0;

    size_t accountedSize = 0;
    for (const auto &obj : parser.getObjects())
    {
        accountedSize += obj.contentSize;

        if (obj.type == "Image")
            breakdown.imagesSize += obj.contentSize;
        else if (obj.type == "Font")
            breakdown.fontsSize += obj.contentSize;
        else if (obj.type == "Metadata")
            breakdown.metadataSize += obj.contentSize;
        else if (obj.type == "Text")
        {
            breakdown.realTextSize += obj.realTextSize;
            breakdown.vectorTextSize += obj.vectorTextSize;
            breakdown.otherTextSize += obj.otherTextSize;
        }
    }

    if (accountedSize < breakdown.totalSize)
        breakdown.textSize += (breakdown.totalSize - accountedSize);
}

void Analyzer::printSummary() const
{
    std::cout << "PDF Size Analysis:\n";
    std::cout << "Total Size: " << breakdown.totalSize << " bytes\n";
    std::cout << "Images: " << breakdown.imagesSize << " bytes\n";
    std::cout << "Fonts: " << breakdown.fontsSize << " bytes\n";
    std::cout << "Metadata: " << breakdown.metadataSize << " bytes\n";
    std::cout << "Text: " << breakdown.textSize << " bytes\n";
    std::cout << "  - Real Text (BT...ET): " << breakdown.realTextSize << " bytes\n";
    std::cout << "  - Vector Text: " << breakdown.vectorTextSize << " bytes\n";
    std::cout << "  - Other Text: " << breakdown.otherTextSize << " bytes\n";
}

const PDFBreakdown &Analyzer::getBreakdown() const
{
    return breakdown;
}

void Analyzer::printFileSize() const
{
    std::cout << "Total PDF File Size: " << breakdown.totalSize << " bytes\n";
}