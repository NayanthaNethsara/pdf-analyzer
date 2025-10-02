#pragma once
#include <string>
#include <vector>

struct PDFObject
{
    size_t startOffset;
    size_t endOffset;
    std::string type;
};

class PDFParser
{
private:
    std::string filePath;
    std::string content;
    std::vector<PDFObject> objects;

public:
    PDFParser(const std::string &path);
    bool load();         // Read file
    void parseObjects(); // Parse obj/endobj
    const std::vector<PDFObject> &getObjects() const;
    size_t getFileSize() const;
};
