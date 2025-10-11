#include "Analyzer.h"
#include <iostream>
#include <iomanip>

Analyzer::Analyzer(const PDFParser &p) : parser(p) {}

void Analyzer::computeBreakdown()
{
    breakdown.totalSize = parser.getFileSize();
    // Initialize all fields in breakdown to avoid reading uninitialized values
    breakdown.imagesSize = 0;
    breakdown.fontsSize = 0;
    breakdown.metadataSize = 0;
    breakdown.textSize = 0;
    breakdown.realTextSize = 0;
    breakdown.vectorTextSize = 0;
    breakdown.otherTextSize = 0;

    size_t accountedSize = 0;
    for (const auto &obj : parser.getObjects())
    {
        accountedSize += obj.contentSize;

        if (obj.type == "Image")
        {
            breakdown.imagesSize += obj.contentSize;
            // Track individual image counts and sizes
            breakdown.imageCount += 1;
            breakdown.imageSizes.push_back(obj.contentSize);
        }
        else if (obj.type == "Font")
        {
            breakdown.fontsSize += obj.contentSize;
        }
        else if (obj.type == "Metadata")
        {
            breakdown.metadataSize += obj.contentSize;
        }
        else if (obj.type == "Text")
        {
            // Make sure text content sizes are accumulated both in specific
            // text sub-categories and in the overall textSize so totals add up.
            breakdown.realTextSize += obj.realTextSize;
            breakdown.vectorTextSize += obj.vectorTextSize;
            breakdown.otherTextSize += obj.otherTextSize;
            // If the parser provides a contentSize for text objects, use it
            // as part of the accounted size and the overall text size.
            breakdown.textSize += obj.contentSize;
        }
        else
        {
            // Unknown object types should still be counted toward accounted size
            // but are considered 'other' text content by default.
            breakdown.otherTextSize += obj.contentSize;
            breakdown.textSize += obj.contentSize;
        }
    }

    if (accountedSize < breakdown.totalSize)
        breakdown.textSize += (breakdown.totalSize - accountedSize);
}

void Analyzer::printSummary() const
{
    std::cout << "PDF Size Analysis:\n";
    std::cout << "Total Size: " << breakdown.totalSize << " bytes\n";
    std::cout << "Images: " << breakdown.imagesSize << " bytes";
    std::cout << " (" << breakdown.imageCount << " images)\n";
    if (!breakdown.imageSizes.empty())
    {
        std::cout << "  Individual image sizes (bytes): ";
        // Print up to first 10 image sizes to avoid excessive output
        size_t toPrint = std::min<size_t>(breakdown.imageSizes.size(), 10);
        for (size_t i = 0; i < toPrint; ++i)
        {
            if (i)
                std::cout << ", ";
            std::cout << breakdown.imageSizes[i];
        }
        if (breakdown.imageSizes.size() > toPrint)
            std::cout << ", ...";
        std::cout << "\n";
    }

    // If parser provided more detailed image metadata, print a table
    const auto &objs = parser.getObjects();
    bool printedHeader = false;
    size_t idx = 0;
    for (const auto &o : objs)
    {
        if (o.type == "Image")
        {
            if (!printedHeader)
            {
                std::cout << "\nImages detail:\n";
                std::cout << "Idx | Name           | Format    | Size (bytes) | WxH\n";
                std::cout << "----+----------------+-----------+--------------+---------\n";
                printedHeader = true;
            }
            ++idx;
            std::cout << std::setw(3) << idx << " | ";
            std::cout << std::left << std::setw(14) << (o.imageName.empty() ? "(unnamed)" : o.imageName) << " | ";
            std::cout << std::left << std::setw(9) << (o.imageFormat.empty() ? "(unknown)" : o.imageFormat) << " | ";
            std::cout << std::right << std::setw(12) << o.contentSize << " | ";
            if (o.width && o.height)
                std::cout << o.width << "x" << o.height << "\n";
            else
                std::cout << "-\n";
        }
    }
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