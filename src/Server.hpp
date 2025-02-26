#ifndef SERVER_HPP
#define SERVER_HPP

//#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <map>
#include "Client.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "CGI.hpp"
#include "Location.hpp"
#include "LimitExcept.hpp"
#include "utils/Utils.hpp"

class Cookie {
private:
    std::string _session_id;
    std::time_t _expiration;
	std::string _session;

public:
	Cookie() : _session_id(""), _expiration(0), _session(""){}
    // Constructor
    Cookie(const std::string& session_id, const std::string &session)
        : _session_id(session_id), _expiration(std::time(0) + (1 * 24 * 60 * 60)), _session(session) {}

    // Getter para session_id
    const std::string& get_session_id() const {
        return _session_id;
    }

    // Getter para expiration
    time_t getExpiration() const {
        return _expiration;
    }
	Cookie& operator=(const Cookie & cook) {
		if (this == &cook)
			return (*this);
		this->_session = cook._session;
		this->_expiration = cook._expiration;
		this->_session_id = cook._session_id;
		return *this;
	}
    // Método para verificar si la cookie ha expirado
    bool isExpired() const {
        return std::time(NULL) > _expiration;
    }
    Cookie& set_session(const std::string & session) {
		_session = session;
		return *this;
    }
    std::string get_session() const {
		return _session;
    }

    // Método para renovar la cookie
    void renew(time_t new_expiration) {
        _expiration = new_expiration;
    }
	bool empty() {
		return (_expiration==0 && _session_id.empty());
	}
};



class Server {

private:
	int _port;
	std::vector<Cookie> _cookies;
	std::string _server_name;
	std::map<int, Client*> _clients;
	std::vector<Location> _locations;
	std::map<int, std::string> _error_pages;
	int _socket_fd;
	size_t _max_clients;
	size_t _timeout;
	size_t _max_req;

	//int epoll_fd;
	//size_t _env_len;
	
	Cookie validate_session_id(std::string &session_id);
	Cookie handle_cookie_session(std::string cookieHeader);
	void execute(Client &client);

public:
	int handle_input_client(int client_fd);
	int handle_output_client(int client_fd);
	Server(int port = 8080, size_t max_clients = 1024, size_t timeout = 1, size_t max_req = 100);
	Server &set_port(const size_t port);
	Server &setSocketFd(const int sock_fd);
	Server &set_server_name(const std::string &server_name);
	Server &setMaxClients(const int max_cients);
	Server &set_error_page(const int code, std::string index);
	Server &addLocation(const Location &location);
	Server &addClient(const Client &cli);
	Server &set_timeout(const size_t timeout);
	Server &set_max_req(const size_t max_req);
	void deleteClients();
	std::string get_error_page_by_key(const int key);
	std::vector<Location> get_locations();
	int getSocketFd() const;
	int getPort() const;
	Client* get_client(int client_fd);
	bool location_empty();
	void deleteClient(const int client_fd);
	int getMaxClients() const;
	bool hasClientTimedOut(const int key_client_fd) {
		std::map<int, Client*>::iterator it = _clients.find(key_client_fd);
		if (it != _clients.end())
			return it->second->has_client_timed_out() > _timeout;
		return false;
	}
	std::string get_server_name() const;
	std::pair <Server*, int> accept_connections(int epoll_fd);

};

#endif