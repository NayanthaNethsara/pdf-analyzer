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
    // Image-specific metadata (filled when type == "Image" when possible)
    std::string imageName;   // resource name or fallback
    std::string imageFormat; // e.g. "JPEG", "PNG", "Flate"
    size_t width = 0;
    size_t height = 0;
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

    // Export image streams to the given directory. Returns vector of output file paths.
    std::vector<std::string> exportImages(const std::string &outDir) const;
};
