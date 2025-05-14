#ifndef HTTPSERVERMANAGER_HPP
#define HTTPSERVERMANAGER_HPP
#include "Server.hpp"
#include <list>
#define MAX_CLIENTS 30

class HttpServerManager {
	private:
		std::vector<int> _srv_sockets;
		int _epoll_fd;
		int _max_events;

		int set_event_action(int client_fd, uint32_t action);
		void handle_epoll();
		int create_socket_fd(int port, int max_clients_srv);
	public:
		HttpServerManager();
		int start();
		void stop();
};

#endif