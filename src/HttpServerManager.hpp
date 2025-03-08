#ifndef HTTPSERVERMANAGER_HPP
#define HTTPSERVERMANAGER_HPP
#include "Server.hpp"
#include <list>
#define MAX_CLIENTS 30

class HttpServerManager {
	private:
		std::map<int, Server*> _sock_srvs;
		std::vector<int> _srv_sockets;
		std::map<int, Server*> _cli_srvs;
		int _epoll_fd;
		int _max_events;

		int set_event_action(int client_fd, uint32_t action);
		void handle_epoll();
		int create_socket_fd(int port);
		std::map<int, Server *>::iterator deleteClient(int client_fd);
	public:
		HttpServerManager();
		int manageIdleClients(struct epoll_event*events, int nfds);
		int start();
		void stop();
};

#endif