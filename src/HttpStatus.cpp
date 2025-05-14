#include "HttpStatus.hpp"

HttpStatus::HttpStatus(): _status() {
	_status[200] = "OK";
	_status[201] = "Created";
	_status[202] = "Accepted";
	_status[204] = "No Content";

	_status[301] = "Moved Permanently";
	_status[302] = "Found";
	_status[304] = "Not Modified";
	_status[307] = "Temporary Redirect";

	_status[400] = "Bad Request";
	_status[401] = "Unauthorized";
	_status[402] = "Payment Required";
	_status[403] = "Forbidden";
	_status[404] = "Not Found";
	_status[405] = "Method Not Allowed";
	_status[406] = "Not Acceptable";
	_status[407] = "Proxy Authentication Required";
	_status[408] = "Request Timeout";
	_status[409] = "Conflict";
	_status[410] = "Gone";
	_status[411] = "Length Required";
	_status[412] = "Precondition Failed";
	_status[413] = "Payload Too Large";
	_status[414] = "URI Too Long";
	_status[415] = "Unsupported Media Type";
	_status[416] = "Range Not Satisfiable";
	_status[417] = "Expectation Failed";
	_status[418] = "I'm a teapot";
	_status[421] = "Misdirected Request";
	_status[422] = "Unprocessable Entity";
	_status[423] = "Locked";
	_status[424] = "Failed Dependency";
	_status[425] = "Too Early";
	_status[426] = "Upgrade Required";
	_status[428] = "Precondition Required";
	_status[429] = "Too Many Requests";
	_status[431] = "Request Header Fields Too Large";
	_status[451] = "Unavailable For Legal Reasons";

	_status[500] = "Internal Server Error";
	_status[501] = "Not Implemented";
	_status[502] = "Bad Gateway";
	_status[503] = "Service Unavailable";
	_status[504] = "Gateway Timeout";
}

HttpStatus::HttpStatus(HttpStatus&) {

}
HttpStatus& HttpStatus::operator=(HttpStatus&){
	return *this;
}
HttpStatus& HttpStatus::getInstance() {
	static HttpStatus instance;
	return instance;
}

std::pair<int, std::string> HttpStatus::getStatusByCode(int code) {
	std::map<int, std::string>::iterator it = _status.find(code);

	if (it != _status.end())
		return std::make_pair(it->first, it->second);
	return std::make_pair(0, "");
}

std::pair<int, std::string> HttpStatus::getStatusByStr(std::string str){ 
	std::map<int, std::string>::iterator it;

	for (it = _status.begin(); it != _status.end() ; it ++) {
		if (it->second == str)
			return std::make_pair(it->first, it->second);
	}
	return std::make_pair(0, "");
}
