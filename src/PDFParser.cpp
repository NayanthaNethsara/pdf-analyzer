#include "PDFParser.h"
#include <fstream>
#include <regex>

PDFParser::PDFParser(const std::string &path) : filePath(path) {}

bool PDFParser::load()
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return false;

    size_t fileSize = file.tellg();
    content.resize(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(&content[0], fileSize);
    file.close();
    return true;
}

void PDFParser::parseObjects()
{
    objects.clear();
    std::regex objRegex(R"((\d+ \d+ obj))");
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

        std::string type = "Text";
        if (objContent.find("/XObject") != std::string::npos)
            type = "Image";
        else if (objContent.find("/Font") != std::string::npos)
            type = "Font";
        else if (objContent.find("/Metadata") != std::string::npos)
            type = "Metadata";

        objects.push_back({objStart, objEnd, type});
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
