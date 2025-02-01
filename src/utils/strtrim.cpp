#include "Utils.hpp"

std::string strtrim(const std::string& str) {
	size_t start;
	size_t end;

	start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos)
		return ("");
	end = str.find_last_not_of(" \t\n\r\f\v");
	return (str.substr(start, end - start + 1));
}