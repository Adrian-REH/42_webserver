#ifndef LOGGER_HPP
#define LOGGER_HPP

#ifndef LOG_LEVEL 
	#define LOG_LEVEL 2
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

class Logger {
public:

	enum LogLevel { INFO, ERROR, WARN, DEBUG };
	static bool enableDebug;

	static void log(LogLevel level, const std::string& module, const std::string& message);

private:
	static std::string logLevelToString(LogLevel level);
};

#endif
