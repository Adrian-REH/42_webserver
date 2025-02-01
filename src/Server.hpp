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


#pragma region Cookie

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

#pragma endregion 

#pragma region Server
class Server {

private:
	int _port;
	std::vector<Cookie> _cookies;
	std::string _server_name;
	std::map<int, Client*> _clients;
	std::vector<Location> _locations;
	int server_fd;
	int epoll_fd;
	struct sockaddr_in _address;
	int _opt;
	int _max_clients;
	size_t _env_len;

	struct epoll_event ev;
	
	Cookie validate_session_id(std::string &session_id);
	bool accept_connections();
	void set_event_action(int client_fd, uint32_t action);
	int handle_input_client(int client_fd);
	int handle_output_client(int client_fd);
	Cookie handle_cookie_session(std::string cookieHeader);
	void execute(Client &client);

public:
	Server(int port = 8080, int opt = 1, int max_clients = 10 );
	Server &set_port(const int &port);
	Server &set_server_name(const std::string &server_name);
	Server &addLocation(const Location &location);
	
	void init();
	void start();
	void event_loop(struct epoll_event events[]);
	void stop();
};
#pragma endregion

#endif