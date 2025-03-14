#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#include "CGI.hpp"
#include "ServerConfig.hpp"
#include "HttpStatus.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "ClientManager.hpp"

class ServerManager {
	private:
		int _epoll_fd;
		enum { _SERVER, _CLIENT, _CGI} type;
		std::map<int, Server *> _cli_srvs;
		std::map<int, Server *> _cgi_srvs;
		std::map<int, Server *> _sock_srvs;
		ServerManager();
		~ServerManager();
		ServerManager& operator=(ServerManager&);
	public:
		static ServerManager& getInstance();
		void set_epoll_fd(int fd);
		int get_epoll_fd() ;
		std::map<int, Server *> get_sock_srvs();
		std::map<int, Server *> get_clis_srvs();
		std::map<int, Server *> get_cgis_srvs();
		Server *get_srv_by_cli(int fd);
		Server *get_srv_by_cgi(int fd);
		Server *get_srv_by_sock(int fd);
		
		std::pair<int, Server*> find_server_type(int socket_fd);
		std::pair<int, Server*> save_server_type(int socket_fd, int type, Server * srv);
		void clear_clis(int epoll_fd);
		void clear_srvs(int epoll_fd);
		std::map<int, Server *>::iterator delete_client(int client_fd, int epoll_fd);

		std::map<int, Server *>::iterator delete_server(int sock_fd, int epoll_fd);
		std::map<int, Server *>::iterator delete_cli_by_cgi(int cgi_fd, int epoll_fd);

		std::map<int, Server *>::iterator delete_cgi(int cgi_fd, int epoll_fd);

		int manageIdleClients(struct epoll_event *events, int nfds, int epoll_fd);
};

#endif