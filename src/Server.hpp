#ifndef SERVER_HPP
#define SERVER_HPP

//#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h> // for epoll_create(), epoll_ctl(), struct epoll_event
#include <map>
#include "Client.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "CGI.hpp"
#include "Location.hpp"
#include "LimitExcept.hpp"
#include "Cookie.hpp"
#include "utils/Utils.hpp"
#include "SessionCookieManager.hpp"
#include "Config.hpp"
class Server {

private:
	int _port;
	size_t _max_clients;
	std::string _server_name;
	std::map<int, Client*> _clients;
	int _socket_fd;

public:
	int handle_input_client(int client_fd);
	int handle_output_client(int client_fd);
	Server(int port = 8080, size_t max_clients = 1024, std::string _server_name = "");
	Server &set_port(const size_t port);
	Server &setSocketFd(const int sock_fd);
	Server &set_server_name(const std::string &server_name);
	Server &setMaxClients(const int max_cients);
	Server &addClient(const Client &cli);
	int getSocketFd() const;
	int getPort() const;
	int getMaxClients() const;
	void deleteClients();
	Client* get_client(int client_fd);
	std::string get_server_name() const;

	
	void deleteClient(const int client_fd);
	bool hasClientTimedOut(const int key_client_fd) {
		ServerConfig srv_conf = Config::getInstance().getServerConfByServerName(_port);
		std::map<int, Client*>::iterator it = _clients.find(key_client_fd);
		if (it != _clients.end())
			return it->second->has_client_timed_out() > srv_conf.get_timeout();
		return false;
	}
	std::pair <Server*, int> accept_connections(int epoll_fd);

};

#endif