#ifndef PARSER_HPP
#define PARSER_HPP
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




class Cookie {
private:
    std::string _session_id;
    time_t _expiration;
	std::string _session;

public:
	Cookie() :_expiration(0) ,_session_id(""), _session(""){}
    // Constructor
    Cookie(const std::string& session_id, time_t expiration, const std::string &session)
        : _session_id(session_id), _expiration(expiration), _session(session) {}

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
	std::map<int, Client*> clients; // TODO: rename this to _clients
	std::vector<Location> _locations;
	int server_fd;
	int epoll_fd;
	struct sockaddr_in address; // TODO: rename this to _address
	int _opt;
	int _max_clients;
	char **_env;
	size_t _env_len;

	Cookie validate_session_id(std::string &session_id) {
		std::vector<Cookie>::iterator it;
		for (it = _cookies.begin(); it != _cookies.end(); it++) {
			if (it->get_session_id() == session_id){
				if (it->isExpired())
					return ( _cookies.erase(it), Cookie());
				return (*it);
			}
		}
		return Cookie();
	}
	
	void add_env( std::string key, std::string value) {
		int i  = -1;
		char **new_env = reinterpret_cast<char**>(malloc(sizeof(char*) * (_env_len + 2)));
		if (!new_env)
			return ;
		for (int i = 0; i < _env_len; i++) {
			new_env[i] = _env[i];
		}
		std::string env_entry = key + "=" + value;
		new_env[_env_len] = strdup(env_entry.c_str());
		if (!new_env[_env_len])
			return ;
		new_env[_env_len + 1] = 0;

/* 		if(_env)
			clear_env();
 */		_env = new_env;
		++_env_len;
	}
	
	bool accept_connections() {
		struct sockaddr_in client_address;
		socklen_t client_len = sizeof(client_address);
		int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
		if (client_fd == -1) {
			perror("Error al aceptar la conexión");
			return 0;
		}

		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
		Client* new_client = new Client(client_fd);
		clients[client_fd] = new_client;
		return 0;
	}

	int handle_client(int client_fd) {

		Client* client = clients[client_fd];
		if (client->receive_data() < 0) {
			std::cerr << "[ERROR] Null client pointer for FD " << client_fd << std::endl;
			return -1;
		}
		std::cout << "[DEBUG] Display_header: " << std::endl;
		client->get_request().display_header();
		std::cout << "[INFO] Executing client request for FD: " << client_fd << std::endl;
		execute(*client);
		std::cout << "[INFO] Successfully completed request for client FD: " << client_fd << std::endl;
		return 0;
	}
	Cookie handle_cookie_session(std::string cookieHeader) {
		Cookie cook;
		std::string sessionID;
		cookieHeader.append(" ");
		if (cookieHeader.find("session_id=") != std::string::npos) {
			sessionID = extractStrBetween(cookieHeader, "session_id=", " ");
			cook = validate_session_id(sessionID);
			if (cook.empty()){
				sessionID = generateSessionID(16); // Genera un nuevo session_id
				return Cookie(sessionID, std::time(0), "invalid");
			}
			else
				return cook;
		}
		sessionID = generateSessionID(16); // No se encontró el session_id, generar uno nuevo
		return (Cookie(sessionID, std::time(0), "invalid"));
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
			add_env("HTTP_COOKIE", ("session=" + cookie.get_session() + "&session_id=" + cookie.get_session_id()));
			std::cout << "[INFO] Executing script: " << path << std::endl;
			//Gestiono la respuesta de la ejecucion del CGI
			rs = CGI(path, method, body, _env).execute();
			if (rs.find("Set-Cookie: session_id:") != std::string::npos)
				cookie.set_session("valid");
			if (!cookie.empty())
				_cookies.push_back(cookie);
			rs_start_line.append(rs);
			std::cout << "[INFO] Sending response to client. Response size: " << rs_start_line.size() << " bytes" << std::endl;
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
	void clear_env() {
		if (_env) {
			int i = -1;
			while (_env[++i])
				free(_env[i]);
			free(_env);
			_env = NULL; // Prevenir liberaciones dobles
		}
		_env_len = 0;
	}
	Server &set_env(char **env) {
       // Contamos las variables de entorno
        size_t count = 0;
        while (env[count] != 0) {
            ++count;
        }
        _env_len = count;
        // Asignamos memoria para _env
        _env = reinterpret_cast<char**>(malloc(sizeof(char*) * (_env_len + 1)));
        if (!_env) {
            // Si falla malloc, dejamos _env como nullptr y devolvemos *this
            _env_len = 0;
            return *this;
        }

        // Copiamos las cadenas del entorno
        for (size_t i = 0; i < _env_len; ++i) {
            _env[i] = strdup(env[i]); // Duplicamos cada cadena
            if (!_env[i]) {
                // Si falla strdup, liberamos toda la memoria y salimos
                clear_env();
                return *this;
            }
        }
        _env[_env_len] = 0; // Terminamos el array con nullptr

        return *this;
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

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(_port);

		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
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
		struct epoll_event ev;
		struct epoll_event events[_max_clients];

		ev.events = EPOLLIN;
		ev.data.fd = server_fd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
			perror("[ERROR] Failed to register server socket with epoll");
			close(server_fd);
			close(epoll_fd);
			exit(EXIT_FAILURE);
		}
		std::cout << "[INFO] Server socket registered with epoll, waiting for events..." << std::endl;
		while (true) {
			int nfds = epoll_wait(epoll_fd, events, _max_clients, -1);
			if (nfds == -1) {
				perror("[ERROR] epoll_wait failed");
				close(server_fd);
				close(epoll_fd);
				exit(EXIT_FAILURE);
			}
			std::cout << "[INFO] Number of events received: " << nfds << std::endl;

			for (int i = 0; i < nfds; ++i) {
				if (events[i].data.fd == server_fd) {
					std::cout << "[INFO] New incoming connection detected" << std::endl;
					if (accept_connections()) {
						std::cout << "[INFO] Connection accepted successfully" << std::endl;
					} else {
						std::cerr << "[WARNING] Failed to accept connection" << std::endl;
					}
				} else {
					std::cout << "[INFO] Handling client with FD: " << events[i].data.fd << std::endl;
					if (handle_client(events[i].data.fd) < 0) {
						std::cerr << "[WARNING] Error handling client with FD: " << events[i].data.fd << std::endl;
						// Cerrar el FD problemático si es necesario
						close(events[i].data.fd);
					}
				}
			}
		}
	}

	void stop() {
		close(server_fd);
		close(epoll_fd);
	}
};



#endif