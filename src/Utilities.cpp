#include "Utilities.h"
#include <sstream>
#include <cctype>
#include <algorithm>

// Convert string to double
double Utils::toDouble(const std::string& s) {
    std::istringstream iss(s);
    double value;
    if (!(iss >> value)) {
        throw std::invalid_argument("Invalid number format: " + s);
    }
    return value;
}

// Trim leading and trailing whitespace from a string
std::string Utils::trim(const std::string& s) {
    size_t start = 0;
    size_t end = s.size() - 1;

    // Trim leading whitespace
    while (start <= end && std::isspace(s[start])) {
        ++start;
    }

    // Trim trailing whitespace
    while (end >= start && std::isspace(s[end])) {
        --end;
    }

    return s.substr(start, end - start + 1);
}

// Check if a string is a valid number
bool Utils::isNumber(const std::string& s) {
    if (s.empty()) return false;

    // Check for a leading '-' (optional)
    size_t start = 0;
    if (s[0] == '-') {
        start = 1;
    }

    bool hasDot = false;
    for (size_t i = start; i < s.size(); ++i) {
        if (std::isdigit(s[i])) {
            continue;
        }
        if (s[i] == '.' && !hasDot) {
            hasDot = true;
            continue;
        }
        return false;  // invalid character found
    }
    return true;
}
