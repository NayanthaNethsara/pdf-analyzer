#include <iostream>
#include "PDFParser.h"
#include "Analyzer.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: pdfanalyzer [--export-images outdir] <file.pdf>\n";
        return 1;
    }

    std::string filePath;
    std::string exportDir;
    // simple CLI: --export-images <dir> before file path
    int i = 1;
    while (i < argc)
    {
        std::string a = argv[i];
        if (a == "--export-images" && i + 1 < argc)
        {
            exportDir = argv[i + 1];
            i += 2;
            continue;
        }
        // assume last non-option is file path
        filePath = a;
        ++i;
    }

    if (filePath.empty())
    {
        std::cerr << "No PDF file provided.\n";
        return 1;
    }

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

    if (!exportDir.empty())
    {
        auto written = parser.exportImages(exportDir);
        if (written.empty())
            std::cout << "No images exported (none found or write error).\n";
        else
        {
            std::cout << "Exported images:\n";
            for (const auto &p : written)
                std::cout << "  " << p << "\n";
        }
    }

    return 0;
}
