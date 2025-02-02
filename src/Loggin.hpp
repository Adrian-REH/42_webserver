#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

class Logger {
public:
	enum LogLevel { INFO, ERROR, DEBUG, WARN };

	static void log(LogLevel level, const std::string& module, const std::string& message) {
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

		// Obtener el PID y nombre de hilo (simulación de hilo)
		logStream << getpid() << " --- [main] ";

		// Nivel de log
		logStream << std::setw(5) << std::left << logLevelToString(level) << " ";

		// Módulo y mensaje
		logStream << module << " : " << message;

		std::cout << logStream.str() << std::endl;
	}

private:
	static std::string logLevelToString(LogLevel level) {
		switch (level) {
			case INFO: return "INFO ";
			case ERROR: return "ERROR";
			case DEBUG: return "DEBUG";
			case WARN: return "WARN ";
		}
		return "UNKNOWN";
	}
};
/* 
// Ejemplo de uso
int main() {
	Logger::log(Logger::INFO, "my.module.Class", "Application started successfully");
	Logger::log(Logger::ERROR, "my.module.Class", "An error occurred");
	Logger::log(Logger::DEBUG, "my.module.Class", "Debugging application");
	return 0;
}
 */