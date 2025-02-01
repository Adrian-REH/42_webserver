#include "Server.hpp"

Server::Server(int port, int opt, int max_clients) : _port(port), _opt(opt), _max_clients(max_clients), _env_len(0){
}

Server &Server::set_port(const int &port) {
	_port=port;
	return *this;
}

Server &Server::set_server_name(const std::string &server_name) {
	_server_name = server_name;
	return *this;
}

Server &Server::addLocation(const Location &location) {
	_locations.push_back(location);
	return *this;
}

void Server::init() {
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

void Server::start() {
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

void Server::event_loop(struct epoll_event events[])
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
			else if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				
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
				if (handle_input_client(events[i].data.fd) < 0) {
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

void Server::stop() {
	_env_len = 0;//TODO: borrar linea
	close(server_fd);
	close(epoll_fd);
}


Cookie Server::validate_session_id(std::string &session_id) {
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
	
bool Server::accept_connections() {
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

void Server::set_event_action(int client_fd, uint32_t action)
{
	ev.events = action | EPOLLET;
	ev.data.fd = client_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
}

int Server::handle_input_client(int client_fd) {
	Client* client;
	if (_clients.find(client_fd) != _clients.end())
		client = _clients[client_fd];
	else {
		std::cerr << "[ERROR] Client for FD " << client_fd << " doesn't exist."<< std::endl;
		return -1;
	}
	if (client->receive_data() < 0)
		return -1;
	//client->get_request().display_header();
	set_event_action(client_fd, EPOLLOUT);
	return 0;
}

int Server::handle_output_client(int client_fd) {
	Client* client = _clients[client_fd];
	std::cout << "[INFO] Executing client request for FD: " << client_fd << std::endl;
	execute(*client);
	set_event_action(client_fd, EPOLLIN);
	return 0;
}

Cookie Server::handle_cookie_session(std::string cookieHeader) {
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

void Server::execute(Client &client) {
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
		/* Define a directory or a file from where the file should be searched (for example,
			if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is
			/tmp/www/pouic/toto/pouet).
		*/
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