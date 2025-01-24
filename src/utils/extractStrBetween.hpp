#ifndef EXTRACTSTRBERWEEN_HPP
#define EXTRACTSTRBERWEEN_HPP
#include <string>
#include <stdexcept>

// TODO: Extract function to hpp cpp
std::string extractStrBetween(const std::string& line, const std::string& init, const std::string& end) {
    size_t startPos = line.find(init);
    if (startPos == std::string::npos) {
        throw std::invalid_argument("Start delimiter not found in line");
    }
    startPos += init.length(); // Mover el inicio justo después del delimitador inicial.

    size_t endPos = line.find(end, startPos);
    if (endPos == std::string::npos) {
        throw std::invalid_argument("End delimiter not found in line");
    }

    return line.substr(startPos, endPos - startPos);
}

std::string extractStrEnd(const std::string& line, const std::string& init) {
    size_t startPos = line.find(init);
    if (startPos == std::string::npos) {
        throw std::invalid_argument("Start delimiter not found in line");
    }
    startPos += init.length(); // Mover el inicio justo después del delimitador inicial.

    return line.substr(startPos, line.length());
}
#endif
