
#include "ServerManager.hpp"
#include <string>

ServerManager::ServerManager(){

}

ServerManager::~ServerManager(){}

ServerManager& ServerManager::operator=(ServerManager&){return *this;}

ServerManager& ServerManager::getInstance(){
	static ServerManager instance;
	return instance;
}
void ServerManager::set_epoll_fd(int fd) {
	_epoll_fd = fd;
}
int ServerManager::get_epoll_fd() {
	return _epoll_fd;
}

std::map<int, Server *> ServerManager::get_sock_srvs() {
	return _sock_srvs;
}
std::map<int, Server *> ServerManager::get_clis_srvs() {
	return _cli_srvs;
}

std::map<int, Server *> ServerManager::get_cgis_srvs() {
	return _cgi_srvs;
}
Server *ServerManager::get_srv_by_cli(int fd) {
	std::map<int, Server *>::iterator it = _cli_srvs.find(fd);
	if (it != _cli_srvs.end())
		return it->second;
	return NULL;
}
Server *ServerManager::get_srv_by_cgi(int fd) {
	std::map<int, Server *>::iterator it = _cgi_srvs.find(fd);
	if (it != _cgi_srvs.end())
		return it->second;
	return NULL;
}
Server *ServerManager::get_srv_by_sock(int fd) {
	std::map<int, Server *>::iterator it = _sock_srvs.find(fd);
	if (it != _sock_srvs.end())
		return it->second;
	return NULL;
}

std::pair<int, Server*> ServerManager::find_server_type(int socket_fd) {
	Server *srv;
	if ((srv = get_srv_by_cli(socket_fd)) != NULL)
		return std::make_pair(_CLIENT, srv);
	else if ((srv = get_srv_by_cgi(socket_fd)) != NULL)
		return std::make_pair(_CGI, srv);
	else if ((srv = get_srv_by_sock(socket_fd)) != NULL)
		return std::make_pair(_SERVER, srv);
	return std::make_pair(-1, srv);
}
std::pair<int, Server*> ServerManager::save_server_type(int socket_fd, int type, Server * srv) {
	Logger::log(Logger::INFO, "ServerManager.cpp", "socket_fd: " + to_string(socket_fd) + ", type: " + to_string(type));
	switch (type)
	{
		case _CLIENT:
				_cli_srvs[socket_fd] = srv;
			break;
		case _CGI:
				_cgi_srvs[socket_fd] = srv;
			break;
		case _SERVER:
				_sock_srvs[socket_fd] = srv;
			break;
		default:
			break;
	}
	return std::make_pair(socket_fd, srv);
}

void ServerManager::clear_clis(int epoll_fd){
	std::map<int, Server *>::iterator it;
	for (it = _cli_srvs.begin();it != _cli_srvs.end(); it++) {
		delete_client(it->first, epoll_fd);
		it = _cli_srvs.begin();
	}
	_cli_srvs.clear();
}
void ServerManager::clear_srvs(int epoll_fd){
	std::map<int, Server *>::iterator it;

	for (it = _sock_srvs.begin();it != _sock_srvs.end(); it++) {
		delete_server(it->first, epoll_fd);
		it->second->deleteClients();
		delete it->second;
		it = _sock_srvs.begin();
	}
	_sock_srvs.clear();
}
void ServerManager::clear_cgis(int epoll_fd){
	std::map<int, Server *>::iterator it;

	for (it = _cgi_srvs.begin();it != _cgi_srvs.end(); it++) {
		this->delete_cgi(it->first, epoll_fd);
		delete it->second;
		it = _cgi_srvs.begin();
	}
	_cgi_srvs.clear();
}

std::map<int, Server *>::iterator ServerManager::delete_client(int client_fd, int epoll_fd){
	std::map<int, Server *>::iterator tmp;
	std::map<int, Server *>::iterator it = _cli_srvs.find(client_fd);
	if (it != _cli_srvs.end()){
		Client * cli = it->second->get_client(client_fd);
		if (cli) {
			std::map<int, CGI*> cgis =  cli->get_cgis();
			std::map<int, CGI*>::iterator it;
			for (it = cgis.begin(); it != cgis.end() ; it++) {
				Logger::log(Logger::DEBUG, "ServerManager.cpp", "Deleting cgi_fd: "+ to_string(it->first) + ", from client_fd: " + to_string(client_fd));
				std::map<int, Server *>::iterator tmp = _cgi_srvs.find(it->first);
				if (tmp != _cgi_srvs.end())
					_cgi_srvs.erase(tmp);
			}
			cli->clear_cgis(epoll_fd);
		}
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
		_cli_srvs[client_fd]->deleteClient(client_fd);
		_cli_srvs.erase(it);
		it = _cli_srvs.begin();
		
	}
	return it;
}
std::map<int, Server *>::iterator ServerManager::delete_server(int sock_fd, int epoll_fd) {
	std::map<int, Server *>::iterator it = _sock_srvs.find(sock_fd);
	if (it != _sock_srvs.end()){
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_fd, NULL);
		close(sock_fd);
		_sock_srvs[sock_fd]->deleteClient(sock_fd);
		_sock_srvs.erase(it);
	}
	return it;
}

void ServerManager::delete_cli_by_cgi(int cgi_fd, int epoll_fd){
	std::map<int, Server *>::iterator it = _cgi_srvs.find(cgi_fd);
	if (it != _cgi_srvs.end()){
		Client* cli = it->second->get_cli_by_pfd(cgi_fd);
		if (cli) {
			std::cout << "ServerManager::delete_cli_by_cgi" << std::endl;
			delete_client(cli->get_socket_fd(), epoll_fd);	
		}
	}
	return ;
}

std::map<int, Server *>::iterator ServerManager::delete_cgi(int cgi_fd, int epoll_fd){
	std::map<int, Server *>::iterator it = _cgi_srvs.find(cgi_fd);
	if (it != _cgi_srvs.end()) {
		Logger::log(Logger::DEBUG, "ServerManager.cpp", "Deleting cgi_fd: "+ to_string(cgi_fd));
		Client* cli = it->second->get_cli_by_pfd(cgi_fd);
		if (cli)
			cli->clear_cgi_by_fd(cgi_fd, epoll_fd);
		_cgi_srvs.erase(it);
	}
	
	return it;
}

int ServerManager::cleanupTimedOutEvents(struct epoll_event *events, int nfds, int epoll_fd) {
	std::map<int, Server*>::iterator it ;
	Server* it_event ;
	for (int i = 0; i < nfds; i++) {
		it_event = this->get_srv_by_cli(events[i].data.fd);
		if (it_event != NULL && it_event->hasClientTimedOut(events[i].data.fd)) {
			Logger::log(Logger::INFO, "ServerManager.cpp", "An event from client_fd: "+ to_string(events[i].data.fd)+", was heard, it is timeout");
			delete_client(events[i].data.fd, epoll_fd);
			events[i].data.fd = 0;
			events[i].events = 0;
			nfds--;
		}
	}
	return nfds;
}

int ServerManager::cleanupTimedOut( int epoll_fd) {
	std::map<int, Server*>::iterator it ;

	for (it = _cli_srvs.begin(); it != _cli_srvs.end(); ) {
		Client * cli = it->second->get_client(it->first);
		if (it->second->hasClientTimedOut(it->first)) {
			Logger::log(Logger::INFO, "ServerManager.cpp", "Connected clients were checked and client_fd: "+ to_string(it->first)+", it is timed out");
			delete_client(it->first, epoll_fd);
			it = _cli_srvs.begin();
		}  else if (cli) {
			Logger::log(Logger::INFO, "ServerManager.cpp", "Checking the client_fd: "+ to_string(it->first)+" cgis");
			cli->killCGITimedOut();
			it++;
		} else {
			it++;
		}
	}
	return 0;
}
