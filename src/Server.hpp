#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <map>
#include "Client.hpp"
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include "CGI.hpp"
#include "utils/endsWith.hpp"
#include "utils/startsWith.hpp"
#include <dirent.h> // Para opendir, readdir, closedir
#include <set>
#include "Location.hpp"
#include "LimitExcept.hpp"
#include <string>
#include <ctime> // Para time_t
#include "utils/randomID.hpp"
#include "utils/extractStrBetween.hpp"

#pragma region utils
std::string strtrim(const std::string& str) {
	size_t start;
	size_t end;

	start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos)
		return ("");
	end = str.find_last_not_of(" \t\n\r\f\v");
	return (str.substr(start, end - start + 1));
}
#pragma endregion

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
	
	Cookie validate_session_id(std::string &session_id) {
		std::vector<Cookie>::iterator it;
		for (it = _cookies.begin(); it != _cookies.end(); it++) {
			if (std::string(it->get_session_id()) == std::string(session_id)){
				if (it->isExpired())
					return ( _cookies.erase(it), Cookie());
				return (*it);
			}
		}
		return Cookie();
	}

	
	bool accept_connections() {
		struct sockaddr_in client_address;
		socklen_t client_len = sizeof(client_address);
		int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
		if (client_fd == -1) {
			perror("Error al aceptar la conexión");
			return 0;
		}
		fcntl(client_fd, F_SETFL, O_NONBLOCK);

		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

		Client* new_client = new Client(client_fd);
		_clients[client_fd] = new_client;
		return 1;
	}

	void set_event_action(int client_fd, uint32_t action)
	{
		if (action == 1)
			ev.events = EPOLLOUT | EPOLLET;
		else if (action == 2)
			ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
	}

	int handle_client(int client_fd) {
		Client* client = _clients[client_fd]; //TODO: Añadir find()
		if (client->receive_data() < 0) {
			std::cerr << "[ERROR] Null client pointer for FD " << client_fd << std::endl;
			return -1;
		}
		std::cout << "[INFO] uuuuu " << client_fd << std::endl;
		client->get_request().display_header();
		std::cout << "[INFO] Changing event action for FD: " << client_fd << std::endl;
		set_event_action(client_fd, 1);
		return 0;
	}

	int handle_output_client(int client_fd) {

		Client* client = _clients[client_fd];
		std::cout << "[INFO] Executing client request for FD: " << client_fd << std::endl;
		execute(*client);
		std::cout << "[INFO] Successfully completed request for client FD: " << client_fd << std::endl;
		set_event_action(client_fd, 2);
		return 0;
	}

	Cookie handle_cookie_session(std::string cookieHeader) {
		Cookie cook;
		std::string sessionID;
		cookieHeader.append(" ");
		if (cookieHeader.find("session_id=") != std::string::npos) {
			sessionID = extractStrEnd(cookieHeader, "session_id=");
			sessionID = strtrim(sessionID);
			cook = validate_session_id(sessionID);

			if (cook.empty()){
				sessionID = generateSessionID(16); // Genera un nuevo session_id
				return Cookie(sessionID, "invalid");
			}
			return cook;
		}
		sessionID = generateSessionID(16); // No se encontró el session_id, generar uno nuevo
		return (Cookie(sessionID, "invalid"));
	}

	void execute(Client &client) {
		Request req = client.get_request();
		std::string path = req.get_path(); // Método para obtener el path del script CGI
		std::string method = req.get_method();    // Método para obtener el método HTTP
		std::string body = req.get_body();        // Método para obtener el cuerpo del request
		try {
			std::cout << "[INFO] Starting CGI execution" << std::endl;
			std::cout << "[DETAIL] Request details: "
					<< "method = " << method 
					<< ", path = " << path 
					<< ", body size = " << body.size() << " bytes" << std::endl;
			std::vector<Location>::iterator it;

			//TODO: Verificar y hacer un algoritmo para manejar los distintos Locations y no ejecutar de mas findScriptPath
			for (it = _locations.begin(); it != _locations.end(); it++) {
				std::cout << "[INFO] Checking location: " << it->get_path() << std::endl;
				path = it->findScriptPath(path);

				if (!path.empty()) {
					std::cout << "[INFO] Matching script found: " << path << std::endl;
					break;
				}
			}
			if (path.empty()) {
				throw std::runtime_error("No script found for the given path");
			}

			std::string rs;
			std::string rs_start_line = "HTTP/1.1 200 OK\r\n";

			//Capturar el id de la Cookie y resolver sus datos, para enviarlo al CGI
			std::string cookie_val = req.get_header_by_key("Cookie");
			std::cout << "[INFO] Verifing Cookie: " << cookie_val << std::endl;
			Cookie cookie = handle_cookie_session(cookie_val);
			//Verifico si la Cookie es valida y lo dejo en env como HTTP_COOKIE="session=invalid/valid"

			std::string http_cookie = "HTTP_COOKIE=" + ("session=" + cookie.get_session() + "; session_id=" + cookie.get_session_id());
			char* env[] = {
				(char*)http_cookie.c_str(),
				NULL
			};
			std::cout << "[INFO] Executing script: " << path << std::endl;
			//Gestiono la respuesta de la ejecucion del CGI
			rs = CGI(path, method, body, env).execute();
			if (rs.find("Set-Cookie: session_id=") != std::string::npos){
				cookie.set_session("valid");
				if (!cookie.empty())
					_cookies.push_back(cookie);
			}
			rs_start_line.append(rs);
			std::cout << "[INFO] Sending response to client. Response size: "<< rs_start_line << " " << rs_start_line.size() << " bytes" << std::endl;
			client.send_response(rs_start_line);
		}
		catch (const std::exception& e)
		{
			std::cerr << "[ERROR] CGI execution failed: " << e.what() << std::endl;
			std::cerr << "[ERROR] Request details: path = " << path 
					<< ", method = " << method 
					<< ", body size = " << body.size() << " bytes" << std::endl;
			client.send_error(500, "Internal Server Error");
		}
	}

public:
	Server(int port = 8080, int opt = 1, int max_clients = 10 ) : _port(port), _opt(opt), _max_clients(max_clients), _env_len(0){
	}

	Server &set_port(const int &port) {
		_port=port;
		return *this;
	}

	Server &set_server_name(const std::string &server_name) {
		_server_name = server_name;
		return *this;
	}

	Server &addLocation(const Location &location) {
		_locations.push_back(location);
		return *this;
	}
	
	void init() {
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd == -1) {
			perror("Error al crear el socket");
			exit(EXIT_FAILURE);
		}

		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(_opt));

		fcntl(server_fd, F_SETFL, O_NONBLOCK);

		_address.sin_family = AF_INET;
		_address.sin_addr.s_addr = INADDR_ANY;
		_address.sin_port = htons(_port);

		if (bind(server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
			perror("Error al hacer bind");
			close(server_fd);
			exit(EXIT_FAILURE);
		}
		if (listen(server_fd, _max_clients) < 0) {
			perror("Error al hacer listen");
			close(server_fd);
			exit(EXIT_FAILURE);
		}
		std::cout << "Listen from: " << "INADDR_ANY" << ":" << _port << std::endl;
		epoll_fd = epoll_create1(0);
		if (epoll_fd == -1) {
			perror("Error al crear epoll");
			close(server_fd);
			exit(EXIT_FAILURE);
		}
		std::cout << "Max-Clients: " << _max_clients << std::endl;
	}

	void start() {
		struct epoll_event events[_max_clients];
		

		ev.events = EPOLLIN;
		ev.data.fd = server_fd;
		
		std::cout << "server_fd: " << server_fd << std::endl;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
			perror("[ERROR] Failed to register server socket with epoll");
			close(server_fd);
			close(epoll_fd);
			exit(EXIT_FAILURE);
		}
		std::cout << "[INFO] Server socket registered with epoll." << std::endl;

		while (true)
			event_loop(events);
	}

	void event_loop(struct epoll_event events[])
	{
		
		while (true) {
			std::cout << "[INFO] Waiting for events (" <<  _port << ")..." << std::endl;
			int nfds = epoll_wait(epoll_fd, events, _max_clients, -1);
			if (nfds == -1) {
				perror("[ERROR] epoll_wait failed");
				close(server_fd);
				close(epoll_fd);
				exit(EXIT_FAILURE);
			}
			std::cout << "[INFO] Number of events received(" <<  _port << "): " << nfds << std::endl;
			
			for (int i = 0; i < nfds; ++i) {
				//TODO: comprobar error control antes que todo
				if (events[i].data.fd == server_fd) {
					std::cout << "[INFO] New incoming connection detected" << std::endl;
					if (accept_connections()) {
						std::cout << "[INFO] Connection accepted successfully" << std::endl;
					} else {
						std::cerr << "[WARNING] Failed to accept connection" << std::endl;
					}
				}
				else if ((events[i].events & EPOLLERR) || 
                        (events[i].events & EPOLLHUP)) {
					
					std::cout << "[ERROR] Failed event "  << std::endl;
					// Client connection closed 
					close(events[i].data.fd);
				} 
				else if ((events[i].events & EPOLLOUT) )//&& events[i].data.fd != server_fd)
				{
					std::cout << "[INFO] Handling output client with FD: " << events[i].data.fd << std::endl;
					handle_output_client(events[i].data.fd);
				}
				else if ((events[i].events & EPOLLIN))// && events[i].data.fd != server_fd)
				{
					std::cout << "[INFO] Handling input client with FD: " << events[i].data.fd << std::endl;
					if (handle_client(events[i].data.fd) < 0) {
						std::cerr << "[WARNING] Error handling client with FD: " << events[i].data.fd << std::endl;
						// Cerrar el FD problemático si es necesario
						close(events[i].data.fd);
					}
					break ;
				}
				else
				{
					std::cerr << "[ERROR] epoll error" << std::endl;
				}
			}
		}
	}

	void stop() {
		_env_len = 0;//TODO: borrar linea
		close(server_fd);
		close(epoll_fd);
	}
};
#pragma endregion

