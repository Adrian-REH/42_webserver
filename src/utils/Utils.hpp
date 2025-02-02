#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdexcept>
#include <deque>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>  // Para time_t
#include <string>
#include <unistd.h>//TODO: borrar
#include <vector>
#include <set>
#include <dirent.h> // Para opendir, readdir, closedir

bool						ends_with(const std::string& str, const std::string& suffix);
std::string					extractStrBetween(const std::string& line, const std::string& init, const std::string& end);
std::string					extractStrEnd(const std::string& line, const std::string& init);
std::string					extractStrStart(const std::string& line, const std::string& end);
int							randomInRange(int min, int max);
std::string					generateSessionID(int length);
std::string					readFd(int fd);
std::deque<std::string>		readFileName(const char *_filename);
std::deque<std::string>		split(const std::string &str, char delimiter);
bool						starts_with(const std::string& str, const std::string& prefix);
std::vector<std::string>	get_all_dirs(const char *dir_path );
std::string					strtrim(const std::string& str);
std::string					to_string(int value);

#endif
