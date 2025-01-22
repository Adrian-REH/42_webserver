#ifndef READFILENAME_HPP
#define READFILENAME_HPP
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

std::deque<std::string> readFileName(const char *_filename)
{
	std::deque<std::string>		content;
	std::string					line;
	std::map<std::string, int>	exchange;
	std::ifstream				inFile(_filename);

	if (!inFile)
		throw std::runtime_error("Error: could not open file");
	while (std::getline(inFile, line))
		content.push_back(line.substr(0, line.find("#")));
	inFile.close();
	return content;
}

#endif