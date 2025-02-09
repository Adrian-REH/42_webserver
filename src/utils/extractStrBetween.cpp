#include "Utils.hpp"

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
        return line;
    }
    startPos += init.length(); // Mover el inicio justo después del delimitador inicial.

    return line.substr(startPos, line.length());
}


std::string extractStrStart(const std::string& line, const std::string& end) {
    size_t endPos = line.rfind(end);
    if (endPos == std::string::npos) {
        return "";
    }

    return line.substr(0, endPos);
}

/**
 * @brief Searches the line backwards until the first ocurrence of the delimiter 
 *          and extracts the string after it
 * 
 * @param line string to search
 * @param end delimiter
 * @return std::string after the delimeter
 */
std::string extractStrREnd(const std::string& line, const std::string& end) {
    size_t endPos = line.rfind(end);
    if (endPos == std::string::npos) {
        return line;
    }
        
    endPos += end.length();
    return line.substr(endPos, line.length());
}
