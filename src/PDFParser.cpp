#include "PDFParser.h"
#include <fstream>
#include <cstring>

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

// binary-safe search
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
        // find " obj"
        size_t objStart = findPattern(content, pos, " obj", 4);
        if (objStart == std::string::npos)
            break;

        // find start of line
        size_t lineStart = objStart;
        while (lineStart > 0 && content[lineStart - 1] != '\n' && content[lineStart - 1] != '\r')
            --lineStart;

        // find "endobj"
        size_t objEnd = findPattern(content, objStart, "endobj", 6);
        if (objEnd == std::string::npos)
            break;
        objEnd += 6;

        size_t contentSize = objEnd - lineStart;
        std::string type = "Text"; // default

        // check for stream content
        size_t streamPos = findPattern(content, lineStart, "stream", 6);
        size_t endStreamPos = findPattern(content, lineStart, "endstream", 9);
        if (streamPos != std::string::npos && endStreamPos != std::string::npos && streamPos < objEnd)
        {
            size_t streamStart = streamPos + 6;
            if (streamStart < content.size() && (content[streamStart] == '\r' || content[streamStart] == '\n'))
                streamStart++;
            contentSize = endStreamPos - streamStart;
        }

        // create snippet for analysis (first 100 bytes or full)
        size_t snippetLen = std::min(static_cast<size_t>(100), objEnd - lineStart);
        std::string snippet(content.begin() + lineStart, content.begin() + lineStart + snippetLen);

        // detect type
        if (snippet.find("/XObject") != std::string::npos)
            type = "Image";
        else if (snippet.find("/Font") != std::string::npos)
            type = "Font";
        else if (snippet.find("/Metadata") != std::string::npos)
            type = "Metadata";

        // initialize text subcategories
        size_t realText = 0, vectorText = 0, otherText = 0;

        if (type == "Text")
        {
            std::string objContent(content.begin() + lineStart, content.begin() + objEnd);

            // Real Text: BT ... ET
            size_t bt = objContent.find("BT");
            size_t et = objContent.find("ET", bt);
            if (bt != std::string::npos && et != std::string::npos && et > bt)
            {
                realText = et + 2 - bt;
            }

            // Vectorized text: path operators (m, l, c)
            if (objContent.find(" m") != std::string::npos ||
                objContent.find(" l") != std::string::npos ||
                objContent.find(" c") != std::string::npos)
            {
                vectorText = objContent.size();
            }

            // Other text: remaining bytes
            if (objContent.size() > realText + vectorText)
                otherText = objContent.size() - (realText + vectorText);
        }

        objects.push_back({lineStart, objEnd, type, contentSize, realText, vectorText, otherText});
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
