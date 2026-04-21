#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace Utils {
    double toDouble(const std::string& s);
    std::string trim(const std::string& s);
    bool isNumber(const std::string& s);
}

#endif