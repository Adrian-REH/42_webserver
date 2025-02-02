#include "Server.hpp"

Server::Server(int port, int max_clients) : _port(port), _max_clients(max_clients), _env_len(0){
}

Server &Server::set_port(const int &port) {
	_port=port;
	return *this;
}

Server &Server::setSocketFd(const int &sock_fd) {
	_socket_fd = sock_fd;
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
Server &Server::addClient(const Client &cli) {
	_clients[cli.get_socket_fd()] = new Client(cli);
	return *this;
}
int Server::getSocketFd() const{
	return _socket_fd;
}
int Server::getPort() const{
	return _port;
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
	return 0;
}

int Server::handle_output_client(int client_fd) {
	Client* client = _clients[client_fd];
	std::cout << "[INFO] Executing client request for FD: " << client_fd << std::endl;
	try {
		execute(*client);

	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
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