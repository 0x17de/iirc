#ifndef IIRC_INIREADER_H
#define IIRC_INIREADER_H


#include <string>
#include <unordered_map>


class IniSection;
class IniReader {
public:
    typedef std::unordered_map<std::string, std::string> configMap;
    typedef std::unordered_map<std::string, configMap> sectionMap;

private:
    sectionMap configuration;
    std::istream& getLine(std::istream& in, std::string& line);

public:
    bool parse(const std::string& fileName);
    IniSection get(std::string key);
};

class IniSection {
    IniReader::configMap* configMap;

public:
    IniSection(IniReader::configMap* configMap);
    std::string get(std::string key);
    operator bool();
};


#endif //IIRC_INIREADER_H
