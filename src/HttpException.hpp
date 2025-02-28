#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP
#include <string>

class HttpException  {
	public:
	class NotAllowedMethodException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "405 Not Allowed Method";
			}
	};
	class NoContentException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "204 No Content";
			}
	};
	
	class NotFoundException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "404 Not Found";
			}
	};
	class BadRequestException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "404 Not Found";
			}
	};
	
	class InternalServerErrorException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "500 Internal Server Error";
			}
	};
	class MovedPermanentlyRedirectionException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "301 Moved Permanently redirection";
			}
	};
};



#endif