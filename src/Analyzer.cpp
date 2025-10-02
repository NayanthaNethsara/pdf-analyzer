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
        size_t objSize = obj.endOffset - obj.startOffset;
        accountedSize += objSize;

        if (obj.type == "Image")
            breakdown.imagesSize += objSize;
        else if (obj.type == "Font")
            breakdown.fontsSize += objSize;
        else if (obj.type == "Metadata")
            breakdown.metadataSize += objSize;
        else
            breakdown.textSize += objSize;
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
}

const PDFBreakdown &Analyzer::getBreakdown() const
{
    return breakdown;
}
