#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP
#include <string>

class HttpException  {
	public:

	class NoContentException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "204 No Content";
			}
	};
	
	class MovedPermanentlyRedirectionException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "301 Moved Permanently redirection";
			}
	};

	class BadRequestException : public std::exception {
		private:
			std::string _message;

		public:
			BadRequestException(std::string message = ""): _message("400 Bad Request : " + message) {}
			virtual ~BadRequestException() throw() {}
			virtual const char* what() const throw() {
				return (_message).c_str();
			}
	};

	class ForbiddenException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "403 Forbidden";
			}
	};


	class NotFoundException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "404 Not Found";
			}
	};

	class NotAllowedMethodException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "405 Not Allowed Method";
			}
	};

	class RequestTimeoutException : public std::exception {
		private:
			std::string _message;
		public:
			RequestTimeoutException(std::string message = ""): _message("408 Request Timeout : " + message) {}
			virtual ~RequestTimeoutException() throw() {}
			virtual const char* what() const throw() {
				return (_message).c_str();
			}
	};
	/*
	//TODO: If the condition is temporary, the server SHOULD include a Retry-
   			After header field to indicate that it is temporary and after what
   			time the client MAY try again.
	*/
	class RequestEntityTooLargeException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "413 Request Entity Too Large";
			}
	};

	class InternalServerErrorException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "500 Internal Server Error";
			}
	};

	class HTTPVersionNotSupportedException : public std::exception {
		public:
			virtual const char* what() const throw() {
				return "505 HTTP Version Not Supported";
			}
	};


	

};



#endif