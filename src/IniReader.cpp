#include "IniReader.h"
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;



std::istream& IniReader::getLine(std::istream& in, string& line) {
    istream::sentry s(in, true);
    stringstream ss;

    bool newLine = true;

    if (s) {
        auto rdbuf = in.rdbuf();

        auto c2 = rdbuf->sbumpc(); // read one character
        decltype(c2) c; // same type as c2

        while (in.good()) {
            c = c2; // current byte
            if (c == EOF) {
                in.setstate(istream::badbit);
                break;
            }
            c2 = rdbuf->sgetc(); // lookahead

            if (c == '\r' || c == '\n') {
                if ((c == '\n' && c2 == '\r') || (c == '\r' && c2 == '\n'))
                    rdbuf->snextc(); // eat second part of linebreak
                break;
            }

            ss << (char)c; // add to output buffer
            rdbuf->snextc(); // eat lookahead
        }
        line = ss.str();
    }

    return in;
}

bool IniReader::parse(const std::string &fileName) {
    ifstream in(fileName);
    if (!in) {
        cerr << "Could not open file." << endl;
        return false;
    }

    configMap* currentConfigMap = nullptr;
    string line;
    size_t lineNo = 0;

    while (in.good()) {
        getLine(in, line);
        ++lineNo;
        if (line.size() == 0 || line.at(0) == '#')
            continue;

        if (line.at(0) == '[') {
            string category = line.substr(1, line.size()-2);
            if (configuration.count(category) > 0) {
                cerr << "Category '" << category << "' appeared twice" << endl;
                return false;
            }
            auto it = configuration.emplace(piecewise_construct, forward_as_tuple(category), forward_as_tuple()); // create new category
            currentConfigMap = &it.first->second;
        }
        else if (line.find('=') != string::npos) {
            if (currentConfigMap == nullptr) {
                cerr << "No category defined yet. See example config." << endl;
                return false;
            }

            istringstream is(line);
            string left, right;
            getline(is, left, '=');
            getline(is, right);

            if (left.at(left.size() - 1) == ' ') {
                left = left.substr(0, left.size() - 1);
                if (right.at(0) == ' ')
                    right = right.substr(1);
            }

            currentConfigMap->emplace(left, right);
        }
        else {
            cerr << "Meaningless content '" << line << "' in line #" << lineNo << endl;
            return false;
        }
    }

    return true;
}

IniSection IniReader::get(std::string key) {
    auto it = configuration.find(key);
    IniReader::configMap* configMap1 = nullptr;

    if (it != configuration.end()) {
        configMap1 = &it->second;
    }

    return IniSection(configMap1);
}

IniSection::IniSection(IniReader::configMap *configMap) {
    this->configMap = configMap;
}

std::string IniSection::get(std::string key) {
    auto it = configMap->find(key);
    if (it == configMap->end())
        return string{};
    return it->second;
}

IniSection::operator bool() {
    return configMap != nullptr;
}
