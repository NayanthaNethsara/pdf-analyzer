#include "PDFParser.h"
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>

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

// Helper to make directory if not exists
static bool ensureDirectory(const std::string &dir)
{
#ifdef _WIN32
    int ret = _mkdir(dir.c_str());
#else
    int ret = mkdir(dir.c_str(), 0755);
#endif
    return (ret == 0 || errno == EEXIST);
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

        // prepare PDFObject and attempt to fill image metadata
        PDFObject pobj;
        pobj.startOffset = lineStart;
        pobj.endOffset = objEnd;
        pobj.type = type;
        pobj.contentSize = contentSize;
        pobj.realTextSize = realText;
        pobj.vectorTextSize = vectorText;
        pobj.otherTextSize = otherText;

        if (type == "Image")
        {
            std::string objContent(content.begin() + lineStart, content.begin() + objEnd);
            // Try to find a Name: /Name <name>
            size_t namePos = objContent.find("/Name");
            if (namePos != std::string::npos)
            {
                size_t start = namePos + 5;
                while (start < objContent.size() && isspace((unsigned char)objContent[start]))
                    ++start;
                size_t end = start;
                while (end < objContent.size() && !isspace((unsigned char)objContent[end]) && objContent[end] != '/' && objContent[end] != '>')
                    ++end;
                pobj.imageName = objContent.substr(start, end - start);
            }

            // Try to detect filter/format
            if (objContent.find("/Filter /DCTDecode") != std::string::npos || objContent.find("/DCTDecode") != std::string::npos)
                pobj.imageFormat = "JPEG";
            else if (objContent.find("/Filter /FlateDecode") != std::string::npos || objContent.find("/FlateDecode") != std::string::npos)
                pobj.imageFormat = "Flate";
            else if (objContent.find("/Filter /JPXDecode") != std::string::npos || objContent.find("/JPXDecode") != std::string::npos)
                pobj.imageFormat = "JPEG2000";

            // Try to find Width and Height
            size_t wpos = objContent.find("/Width");
            if (wpos != std::string::npos)
            {
                size_t start = wpos + 6;
                while (start < objContent.size() && isspace((unsigned char)objContent[start]))
                    ++start;
                size_t end = start;
                while (end < objContent.size() && isdigit((unsigned char)objContent[end]))
                    ++end;
                if (end > start)
                    pobj.width = std::stoul(objContent.substr(start, end - start));
            }
            size_t hpos = objContent.find("/Height");
            if (hpos != std::string::npos)
            {
                size_t start = hpos + 7;
                while (start < objContent.size() && isspace((unsigned char)objContent[start]))
                    ++start;
                size_t end = start;
                while (end < objContent.size() && isdigit((unsigned char)objContent[end]))
                    ++end;
                if (end > start)
                    pobj.height = std::stoul(objContent.substr(start, end - start));
            }
        }

        objects.push_back(pobj);
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

std::vector<std::string> PDFParser::exportImages(const std::string &outDir) const
{
    std::vector<std::string> outFiles;
    if (!ensureDirectory(outDir))
        return outFiles;

    size_t idx = 0;
    for (const auto &o : objects)
    {
        if (o.type != "Image")
            continue;

        // attempt to find stream region inside the object
        size_t objStart = o.startOffset;
        size_t objEnd = o.endOffset;
        size_t streamPos = findPattern(content, objStart, "stream", 6);
        size_t endStreamPos = findPattern(content, objStart, "endstream", 9);
        size_t streamStart = objStart;
        size_t streamLen = o.contentSize;
        if (streamPos != std::string::npos && endStreamPos != std::string::npos && streamPos < objEnd)
        {
            streamStart = streamPos + 6;
            if (streamStart < content.size() && (content[streamStart] == '\r' || content[streamStart] == '\n'))
                streamStart++;
            streamLen = endStreamPos - streamStart;
        }

        std::string ext = "bin";
        if (o.imageFormat == "JPEG")
            ext = "jpg";
        else if (o.imageFormat == "PNG")
            ext = "png";
        else if (o.imageFormat == "JPEG2000")
            ext = "jp2";
        else if (o.imageFormat == "Flate")
            ext = "dat";

        ++idx;
        std::ostringstream fname;
        fname << outDir << "/image-" << idx;
        if (!o.imageName.empty())
            fname << "-" << o.imageName;
        fname << "." << ext;

        std::ofstream ofs(fname.str(), std::ios::binary);
        if (!ofs)
            continue;
        ofs.write(reinterpret_cast<const char *>(content.data() + streamStart), streamLen);
        ofs.close();
        outFiles.push_back(fname.str());
    }

    return outFiles;
}
