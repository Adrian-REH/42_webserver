
#include "Utils.hpp"


std::deque<std::string> readFileName(const char *_filename)
{
	std::deque<std::string>		content;
	std::string					line;
	std::ifstream				inFile(_filename);

	if (!inFile)
		throw std::runtime_error("Error: could not open file");
	while (std::getline(inFile, line))
		content.push_back(line.substr(0, line.find("#")));
	inFile.close();
	return content;
}

std::string readFileNameToStr(const char *_filename)
{
	std::string					content;
	std::string					line;
	std::ifstream				inFile(_filename);

	if (!inFile)
		throw std::runtime_error("Error: could not open file");
	while (std::getline(inFile, line))
		content += (line + "\n");
	inFile.close();
	return content;
}