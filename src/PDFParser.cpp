#include "PDFParser.h"
#include <fstream>
#include <cstring> // for memcmp

PDFParser::PDFParser(const std::string &path) : filePath(path) {}

bool PDFParser::load()
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return false;

    size_t fileSize = file.tellg();
    content.resize(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(content.data()), fileSize);
    file.close();

    return true;
}

// Binary-safe helper to find a pattern in content
size_t findPattern(const std::vector<unsigned char> &data, size_t start,
                   const char *pattern, size_t patternLen)
{
    for (size_t i = start; i + patternLen <= data.size(); ++i)
    {
        if (memcmp(&data[i], pattern, patternLen) == 0)
            return i;
    }
    return std::string::npos;
}

void PDFParser::parseObjects()
{
    objects.clear();
    size_t pos = 0;

    while (pos < content.size())
    {
        // Look for " obj"
        size_t objStart = findPattern(content, pos, " obj", 4);
        if (objStart == std::string::npos)
            break;

        // Find start of line (to get object number, optional)
        size_t lineStart = objStart;
        while (lineStart > 0 && content[lineStart - 1] != '\n' && content[lineStart - 1] != '\r')
            --lineStart;

        // Look for "endobj"
        size_t objEnd = findPattern(content, objStart, "endobj", 6);
        if (objEnd == std::string::npos)
            break;
        objEnd += 6;

        size_t contentSize = objEnd - lineStart;
        std::string type = "Text"; // default

        // Check object content for type keywords
        size_t streamPos = findPattern(content, lineStart, "stream", 6);
        size_t endStreamPos = findPattern(content, lineStart, "endstream", 9);
        if (streamPos != std::string::npos && endStreamPos != std::string::npos && streamPos < objEnd)
        {
            size_t streamStart = streamPos + 6;
            // skip possible newline
            if (streamStart < content.size() && (content[streamStart] == '\r' || content[streamStart] == '\n'))
                streamStart++;
            contentSize = endStreamPos - streamStart;
        }

        // Detect type by keywords
        size_t objLen = objEnd - lineStart;
        if (objLen > 0)
        {
            std::string objSnippet;
            if (objLen > 100)
                objLen = 100; // first 100 bytes to check
            objSnippet = std::string(content.begin() + lineStart, content.begin() + lineStart + objLen);

            if (objSnippet.find("/XObject") != std::string::npos)
                type = "Image";
            else if (objSnippet.find("/Font") != std::string::npos)
                type = "Font";
            else if (objSnippet.find("/Metadata") != std::string::npos)
                type = "Metadata";
        }

        objects.push_back({lineStart, objEnd, type, contentSize});
        pos = objEnd;
    }
}

const std::vector<PDFObject> &PDFParser::getObjects() const
{
    return objects;
}

size_t PDFParser::getFileSize() const
{
    return content.size();
}
