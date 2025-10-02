#include <iostream>
#include "PDFParser.h"
#include "Analyzer.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: pdfanalyzer <file.pdf>\n";
        return 1;
    }

    std::string filePath = argv[1];

    PDFParser parser(filePath);
    if (!parser.load())
    {
        std::cerr << "Failed to load PDF.\n";
        return 1;
    }

    parser.parseObjects();

    Analyzer analyzer(parser);
    analyzer.computeBreakdown();
    analyzer.printSummary();

    return 0;
}
