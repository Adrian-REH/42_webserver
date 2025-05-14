#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <string>
#include <map>

class HttpStatus {
	private:
		std::map<int, std::string> _status;
		HttpStatus();
		HttpStatus(HttpStatus&);
		HttpStatus& operator=(HttpStatus&);
	public:
		static HttpStatus& getInstance();
		std::pair<int, std::string> getStatusByCode(int);
		std::pair<int, std::string> getStatusByStr(std::string);
};

#endif