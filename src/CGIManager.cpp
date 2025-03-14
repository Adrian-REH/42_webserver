#include "CGIManager.hpp"
#include <sys/epoll.h>


CGIManager::CGIManager(): _pfd_cgi() {}

CGIManager::~CGIManager() {}

CGIManager& CGIManager::operator=(CGIManager&)
{return *this;}

CGIManager & CGIManager::getInstance() {
	static CGIManager instance;
	return instance;
}

/** @brief Se llama continuamente para verificar si esta en Timeout el fd y lo mata en caso de ser necesario
 * 
 */
int CGIManager::timeout(int pfd) {
	std::map<int, CGI*>::iterator it = _pfd_cgi.find(pfd);
	if (it != _pfd_cgi.end())
		return it->second->istimeout();
	return 0;
}

CGI *CGIManager::save_cgi_by_pfd(std::pair<int, CGI*> pfd_cgi){
	_pfd_cgi[pfd_cgi.first] = pfd_cgi.second;
	return pfd_cgi.second;
}
int CGIManager::kill_cgi_by_pfd(int pfd, int epoll_fd){
	std::map<int, CGI*>::iterator it = _pfd_cgi.find(pfd);
	if (it != _pfd_cgi.end()) {
		it->second->cgi_kill();
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
		return 0;
	}
	return -1;
}
CGI *CGIManager::get_cgi_by_pfd(int pfd){
	std::map<int, CGI*>::iterator it = _pfd_cgi.find(pfd);
	if (it != _pfd_cgi.end()) {
		return (it->second);
	}
	return NULL;
}

