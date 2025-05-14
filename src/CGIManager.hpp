#ifndef CGIMANAGER_HPP
#define CGIMANAGER_HPP
#include "CGI.hpp"
#include "ServerConfig.hpp"
#include "HttpStatus.hpp"

class CGIManager {
	private:
		std::map<int, CGI*> _pfd_cgi;// Pipe de respuesta en base a CGI
		CGIManager();
		~CGIManager();
		CGIManager& operator=(CGIManager&);
	public:
		static CGIManager& getInstance();
		CGI* save_cgi_by_pfd(std::pair<int, CGI*>);
		int kill_cgi_by_pfd(int, int);
		int timeout(int);
		CGI *get_cgi_by_pfd(int);
};

#endif