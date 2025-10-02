#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

struct PDFBreakdown
{
    size_t totalSize = 0;
    size_t imagesSize = 0;
    size_t fontsSize = 0;
    size_t metadataSize = 0;
    size_t textSize = 0;
};

// Simple structure to hold object info
struct PDFObject
{
    size_t startOffset;
    size_t endOffset;
    std::string type;
};

std::vector<PDFObject> parseObjects(const std::string &content)
{
    std::vector<PDFObject> objects;
    std::regex objRegex(R"((\d+ \d+ obj))"); // matches object start e.g., 12 0 obj
    std::sregex_iterator it(content.begin(), content.end(), objRegex);
    std::sregex_iterator end;

    while (it != end)
    {
        size_t objStart = it->position();
        ++it;
        size_t objEnd = content.find("endobj", objStart);
        if (objEnd == std::string::npos)
            break;
        objEnd += 6; // include "endobj"

        std::string objContent = content.substr(objStart, objEnd - objStart);

        std::string type = "Text"; // default
        if (objContent.find("/XObject") != std::string::npos)
            type = "Image";
        else if (objContent.find("/Font") != std::string::npos)
            type = "Font";
        else if (objContent.find("/Metadata") != std::string::npos)
            type = "Metadata";

        objects.push_back({objStart, objEnd, type});
    }
    return objects;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: pdfanalyzer <file.pdf>\n";
        return 1;
    }

    std::string filePath = argv[1];
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return 1;
    }

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content(fileSize, '\0');
    file.read(&content[0], fileSize);
    file.close();

    PDFBreakdown breakdown;
    breakdown.totalSize = fileSize;

    auto objects = parseObjects(content);
    size_t accountedSize = 0;

    for (auto &obj : objects)
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

    // Any leftover bytes (outside objects) also counted as text
    if (accountedSize < fileSize)
        breakdown.textSize += (fileSize - accountedSize);

    // Print results
    std::cout << "PDF Size Analysis:\n";
    std::cout << "Total Size: " << breakdown.totalSize << " bytes\n";
    std::cout << "Images: " << breakdown.imagesSize << " bytes\n";
    std::cout << "Fonts: " << breakdown.fontsSize << " bytes\n";
    std::cout << "Metadata: " << breakdown.metadataSize << " bytes\n";
    std::cout << "Text: " << breakdown.textSize << " bytes\n";

    return 0;
}
