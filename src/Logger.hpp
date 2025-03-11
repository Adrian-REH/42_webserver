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

	static void log(LogLevel level, const std::string& module, const std::string& message) {
		if (LOG_LEVEL  < level)
			return ;
		std::ostringstream logStream;
		
		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);
		struct timeval tv;
		gettimeofday(&tv, NULL);

		logStream << (now->tm_year + 1900) << "-"
				  << std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << "-"
				  << std::setw(2) << std::setfill('0') << now->tm_mday << " "
				  << std::setw(2) << std::setfill('0') << now->tm_hour << ":"
				  << std::setw(2) << std::setfill('0') << now->tm_min << ":"
				  << std::setw(2) << std::setfill('0') << now->tm_sec << "."
				  << std::setw(3) << std::setfill('0') << (tv.tv_usec / 1000) << " ";

		logStream << getpid() << " --- [main] ";

		logStream << std::setw(5) << std::left << logLevelToString(level) << " ";

		logStream << module << " : " << message;

		std::cout << logStream.str() << std::endl;
	}

private:
	static std::string logLevelToString(LogLevel level) {
		switch (level) {
			case INFO: return "INFO ";
			case ERROR: return "ERROR";
			case WARN: return "WARN ";
			case DEBUG: return "DEBUG";
		}
		return "UNKNOWN";
	}
};

#endif
