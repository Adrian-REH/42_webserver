#include "Server.hpp"
#include "Logger.hpp"

Server::Server(int port, size_t max_clients, size_t timeout, size_t max_req) : _port(port), _max_clients(max_clients), _timeout(timeout),_max_req(max_req) { //,_max_clients(max_clients), _env_len(0){
}

Server &Server::set_max_req(const size_t max_req) {

	if (this->_max_req && max_req)
		this->_max_req = (max_req > 100) ? 100: max_req;
	return *this;
}

Server &Server::set_timeout(const size_t timeout) {
	if (this->_timeout && timeout)
	this->_timeout = (timeout > 5) ? 5: timeout;
	return *this;
}

Server &Server::set_port(const size_t port) {

	_port=port;
	return *this;
}

Server &Server::setSocketFd(const int sock_fd) {
	_socket_fd = sock_fd;
	return *this;
}
Server &Server::set_server_name(const std::string &server_name) {
	_server_name = server_name;
	return *this;
}

Server &Server::setMaxClients(const int max_cients) {
	_max_clients = max_cients;
	return *this;
}

Server &Server::set_error_page(const int code, std::string index) {
	_error_pages[code] = index;
	return *this;
}

Server &Server::addLocation(const Location &location) {
	_locations.push_back(location);
	return *this;
}

Server &Server::addClient(const Client &cli) {
	_clients[cli.get_socket_fd()] = new Client(cli);
	return *this;
}


std::vector<Location> Server::get_locations() {
	return _locations;
}
int Server::getSocketFd() const{
	return _socket_fd;
}

int Server::getPort() const{
	return _port;
}

int Server::getMaxClients() const{
	return _max_clients;
}

std::string Server::get_server_name() const {
	return _server_name;
}

std::string Server::get_error_page_by_key(const int key) {
	return _error_pages[key];
}

void Server::deleteClient(const int client_fd) {
	Logger::log(Logger::DEBUG, "Server.cpp", "Deleting client_fd: "+ to_string(client_fd));

	std::map<int, Client *>::iterator it=  _clients.find(client_fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
	}
}

void Server::deleteClients() {
	Logger::log(Logger::DEBUG, "Server.cpp", "Deleting all clients: ");

	std::map<int, Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end() ; it++) {
		delete it->second;
	}
	_clients.clear();
}


std::pair<std::string, std::string> client_info(struct sockaddr_in client_address) {
    char client_ip[INET_ADDRSTRLEN];  // Buffer para la IP
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);

    int client_port = ntohs(client_address.sin_port); // Convertir a formato legible

	std::string ip(client_ip);
	return std::make_pair(ip, to_string(client_port));
}

std::pair<Server*, int> Server::accept_connections(int epoll_fd) {
	struct sockaddr_in client_address;
	struct epoll_event ev;
	socklen_t client_len = sizeof(client_address);
	int client_fd = accept(_socket_fd, (struct sockaddr *)&client_address, &client_len);
	if (client_fd == -1) {
		Logger::log(Logger::WARN, "Server.cpp", "Error al aceptar la conexion");
		return std::make_pair(this, -1);
	}
	std::pair<std::string, std::string> data_cli = client_info(client_address);
	size_t n_clients = _clients.size();
	if (n_clients >= _max_clients) {
		Logger::log(Logger::ERROR, "Server.cpp", "¡Servidor Lleno!, n_clients: " + to_string(n_clients) + ", max_clients: " + to_string(_max_clients));
		Logger::log(Logger::DEBUG, "Server.cpp", "The client was rejected: IP: " + data_cli.first + "Port + " + data_cli.second );
		if (client_fd != -1) {
			// Enviar la respuesta nRetry-After 5 seg
			send(client_fd, "HTTP/1.1 503 Service Unavailable\r\nRetry-After: 5\r\n\r\n", 56, 0);
			close(client_fd);
		}
		return std::make_pair(this, -1);
	}
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = client_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
	_clients[client_fd] = new Client(client_fd);
	_clients[client_fd]->set_ip(data_cli.first);
	_clients[client_fd]->set_port(data_cli.second);

	Logger::log(Logger::DEBUG, "Server.cpp", "The client was acepted: IP:PORT " + data_cli.first + ":" + data_cli.second + " fd: " + to_string(client_fd));
	return std::make_pair(this, client_fd);
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
Client* Server::get_client(int client_fd) {
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	Client* client = NULL;
	if (it != _clients.end())
		client = it->second;
	return client;
}

int Server::handle_input_client(int client_fd) {
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	Client* client = NULL;
	if (it != _clients.end())
		client = it->second;
	else {
		Logger::log(Logger::ERROR,"Server.cpp", "Client for FD" + to_string(client_fd) + " doesn't exist.");
		return -1;
	}

	try {
		if (client->receive_data() < 0)
			return -1;
	} catch (const std::exception &e)
	{
		std::cerr <<"Exception: " << e.what() << std::endl;
		//return -1;
	}
	return 0;
}

int Server::handle_output_client(int client_fd) {
	Client* client = _clients[client_fd];
	Logger::log(Logger::INFO,"Server.cpp", "Executing read and send request for client_fd: " + to_string(client_fd));
	try {
		execute(*client);
	} catch (std::exception &e) {
		client->send_error(500, "Internal Server Error");
		std::string val(e.what());
		Logger::log(Logger::ERROR,"Server.cpp",  e.what());
		return -1;
	}
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

std::string generate_index_html(std::vector<std::string> files, std::string dir_path) {
	std::string index_file;
    // Escribir el encabezado HTML
	index_file = "Content-Type: text/html\r\n\r\n";
    index_file.append("<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n<title>Index of ");
	index_file.append(dir_path);
	index_file.append("</title>\n</head>\n<body>\n");
    index_file.append( "<h1>Index of ");
	index_file.append(dir_path);
	index_file.append("</h1>\n<ul>\n");

    // Leer los archivos y directorios
	std::vector<std::string>::iterator it;
    for (it = files.begin(); it != files.end(); it ++) {
        std::string entry_path = *it;
        // Ignorar los directorios "." y ".."
        if (entry_path == "." || entry_path == "..") {
            continue;
        }
        // Escribir cada archivo/directorio en la lista HTML
        index_file.append( "<li><a href=\"");
		index_file.append(entry_path);
		index_file.append("\">");

		std::string entry_name(entry_path);
		entry_name = extractStrREnd(entry_name, "/");
	
		index_file.append(entry_name);
		index_file.append("</a></li>\n");
    }

    // Escribir el pie de página HTML
    index_file.append("</ul>\n</body>\n</html>\n");
	return index_file;
}

void Server::execute(Client &client) {
	Request	req = client.get_request();
	std::string path = req.get_path(); // Método para obtener el path del script CGI
	std::string method = req.get_method();    // Método para obtener el método HTTP
	std::string body = req.get_body();        // Método para obtener el cuerpo del request
	bool autoindex = false;
	std::vector<std::string> files;

	if (req.get_state() <  3) // Server needs to wait to receive the full request since is nonblocking
	{
		Logger::log(Logger::INFO, "Server.cpp", "PENDING TO FINISH THE REQUEST");
		return ;
	}

	try {
		Logger::log(Logger::INFO,"Server.cpp", "Starting CGI execution.");
		std::string rs;
		std::string rs_start_line = "HTTP/1.1 200 OK\r\n";
		Logger::log(Logger::DEBUG,"Server.cpp", "Request details: method: '" + method + "', path: '" + path + "', body size: " + to_string(body.size()) + "bytes");
		std::vector<Location>::iterator it;
		std::map<std::string, Location>::iterator it_map;
		bool is_cgi = false;
		std::string path_tmp;
		
		//Rechazar u Aceptar Keep-Alive:
		std::string connection = req.get_header_by_key("Connection");
		if (connection.empty()){
			rs_start_line.append("Connection: close\r\n");
		}
		else if (_timeout > 0 && !connection.empty() && connection.compare("keep-alive") && !client.has_max_req(_max_req) ) {
			rs_start_line.append("Connection: Keep-Alive\r\n");
			rs_start_line.append("Keep-Alive: timeout=" + to_string(_timeout) + ", " + "max= " + to_string(_max_req) + "\r\n");
		}
		for (it = _locations.begin(); it != _locations.end(); it++) {
			Logger::log(Logger::DEBUG,"Server.cpp","urlRel:" + path + ", loc_path:" + it->get_path());
			if (starts_with(path, it->get_path()))
			{
				Logger::log(Logger::INFO,"Server.cpp", "Checking location: " + it->get_path());
				try {
					if (it->findScriptPath(path, path_tmp) == 0)
						break;
					else //if (it->findScriptPath(path, path_tmp) == 1)
					{
						autoindex = it->get_auto_index();
						files = it->get_files();
						break ;
					}
				} catch (const std::exception &e) {
					std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
				}
			}
		}

		if (it != _locations.end() && !it->get_limit_except().isMethodAllowed(method))
		{
			Logger::log(Logger::INFO,"Server.cpp", "Method not allowed: " + method);
			rs = "HTTP/1.1 405 Method Not Allowed";
			Logger::log(Logger::INFO,"Server.cpp", "Method not allowed: " + method);
			rs = "HTTP/1.1 405 Method Not Allowed";
			//Llamar a una clase que aloje todos los errores: HttpStatusStore
			if (method != "HEAD")
			{
				rs += "\nContent-Type: text/html\r\n\r\n";
				std::ifstream	inFile("html/405.html");
				if (!inFile)
					std::cout << "ERROR inFile"<<std::endl;
				std::string line;
				while (std::getline(inFile, line))
					rs += line + "\n";
				inFile.close();
			} else 
				rs += "\r\n\r\n";
			//std::cout << "[rs]: " << rs << std::endl;
			client.send_response(rs);
			return ;
		}

		if (ends_with(path_tmp, ".html"))
		{
			path_tmp.erase(0,1);
			Logger::log(Logger::INFO,"Server.cpp", "HTML file: " + path_tmp);
			
			
			rs = "Content-Type: text/html\r\n\r\n";
			if (method != "HEAD")
			{
				std::ifstream	inFile(path_tmp.c_str());
				if (!inFile)
					std::cout << "ERROR inFile"<<std::endl;
				std::string line;
				while (std::getline(inFile, line))
					rs += line + "\n";
				//std::cout << rs << std::endl;
				inFile.close();
			} else 
				rs_start_line = "HTTP/1.1 204 No Content\r\n"; 
			
		}
		else
		{
			size_t dot_pos = path_tmp.rfind('.');
			if ((dot_pos != std::string::npos) && (dot_pos != path.length() - 1))
			{
				Logger::log(Logger::INFO,"Server.cpp", "Matching script found: " + path_tmp);
				path = path_tmp;
				is_cgi = true;
			}
			else if (autoindex) // Mostrar contenido dir
			{
				//TODO: no script encontrado pero tiene autoindex -> no error
				Logger::log(Logger::INFO,"Server.cpp", "Generating index: " + path_tmp);
				rs = generate_index_html(files, path_tmp);
				//std::cout << rs << std::endl;
				rs_start_line.append(rs);
				client.send_response(rs_start_line);
				return ;
			}
			else {
				Logger::log(Logger::ERROR,"Server.cpp", "Throwing exception " + path_tmp);
				throw std::runtime_error("No script found for the given path");
			}
		
		}

		//Capturar el id de la Cookie y resolver sus datos, para enviarlo al CGI
		std::string cookie_val = req.get_header_by_key("Cookie");
		Logger::log(Logger::INFO,"Server.cpp", "Verifing Cookie: " + cookie_val);
		Cookie cookie = handle_cookie_session(cookie_val);
		//Verifico si la Cookie es valida y lo dejo en env como HTTP_COOKIE="session=invalid/valid"
		
		if (is_cgi)
		{
			std::string http_cookie = "HTTP_COOKIE=" + ("session=" + cookie.get_session() + "; session_id=" + cookie.get_session_id());
			std::string req_m = "REQUEST_METHOD=" + method;
			std::string content_length  = "CONTENT_LENGTH=" + req.get_header_by_key("Content-Length"); // Como lee Webserver por chunked el body entonces no hare que CGI se encarge de leerlo.
			std::string query_string = "QUERY_STRING=" + req.get_query_string();
			std::string script_name = "SCRIPT_NAME=" + path;
			std::string content_type = "CONTENT_TYPE=" + req.get_header_by_key("Content-Type");
			std::string server_name = "SERVER_NAME=" + req.get_header_by_key("Host");
			std::string remote_addr = "REMOTE_ADDR=" + client.get_ip();
			std::string server_port = "SERVER_PORT=" + to_string(_port);
			std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
			std::string http_user_agent = "HTTP_USER_AGENT=" + req.get_header_by_key("User-Agent");
			std::string http_referer = "HTTP_REFERER=" + req.get_header_by_key("Referer");

			char* env[] = {
				(char*)http_cookie.c_str(),
				(char*)req_m.c_str(),
				(char*)content_length .c_str(),
				(char*)query_string .c_str(),
				(char*)script_name .c_str(),
				(char*)content_type .c_str(),
				(char*)server_name .c_str(),
				(char*)remote_addr .c_str(),
				(char*)server_port .c_str(),
				(char*)server_protocol .c_str(),
				(char*)http_user_agent .c_str(),
				(char*)http_referer .c_str(),
				NULL
			};
			Logger::log(Logger::INFO,"Server.cpp", "Executing script: " + path);
			//Gestiono la respuesta de la ejecucion del CGI
			CGI cgi(path, method, body, env);
			cgi.parse_request_details(req.get_headers());
			rs = cgi.execute();
			Logger::log(Logger::INFO,"Server.cpp", "Finished execut script: ");
		}
		
		// Agrego Set-cookie en caso de que lo requiera
		if (rs.find("Set-Cookie: session_id=") != std::string::npos){
			cookie.set_session("valid");
			if (!cookie.empty())
				_cookies.push_back(cookie);
		}
		rs_start_line.append(rs);
		Logger::log(Logger::INFO,"Server.cpp", "Sending response to client. Response size: " + to_string(rs_start_line.size()) + "bytes");
		client.send_response(rs_start_line);
	} catch(std::exception &e) {
		Logger::log(Logger::ERROR,"Server.cpp", e.what());
		client.send_error(404, "Not Found");
	}
}