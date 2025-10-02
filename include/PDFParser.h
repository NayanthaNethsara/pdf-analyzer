#pragma once
#include <string>
#include <vector>

struct PDFObject
{
    size_t startOffset;
    size_t endOffset;
    std::string type;      // "Image", "Font", "Metadata", "Text"
    size_t contentSize;    // total bytes
    size_t realTextSize;   // BT...ET
    size_t vectorTextSize; // path-based text
    size_t otherTextSize;  // leftover
};

class PDFParser
{
private:
    std::string filePath;
    std::vector<unsigned char> content;
    std::vector<PDFObject> objects;

public:
    PDFParser(const std::string &path);
    bool load();         // Load PDF into memory
    void parseObjects(); // Parse objects safely for binary PDFs
    const std::vector<PDFObject> &getObjects() const;
    size_t getFileSize() const;
};
