#include "Server.hpp"
#include "Loggin.hpp"

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
		Logger::log(Logger::ERROR,"Server.cpp", "Client for FD" + to_string(client_fd) + " doesn't exist.");
		return -1;
	}
	if (client->receive_data() < 0)
		return -1;
	return 0;
}

int Server::handle_output_client(int client_fd) {
	Client* client = _clients[client_fd];
	Logger::log(Logger::INFO,"Server.cpp", "Executing read request for client_fd: " + to_string(client_fd));
	try {
		execute(*client);

	} catch (std::exception &e) {
		Logger::log(Logger::ERROR,"Server.cpp",  e.what());
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
        std::string entry_name = *it;
        // Ignorar los directorios "." y ".."
        if (entry_name == "." || entry_name == "..") {
            continue;
        }
        // Escribir cada archivo/directorio en la lista HTML
        index_file.append( "<li><a href=\"");
		index_file.append(entry_name);
		index_file.append("\">");
		index_file.append(entry_name);
		index_file.append("</a></li>\n");
    }

    // Escribir el pie de página HTML
    index_file.append("</ul>\n</body>\n</html>\n");
	return index_file;
}

void Server::execute(Client &client) {
	Request req = client.get_request();
	std::string path = req.get_path(); // Método para obtener el path del script CGI
	std::string method = req.get_method();    // Método para obtener el método HTTP
	std::string body = req.get_body();        // Método para obtener el cuerpo del request
	try {
		Logger::log(Logger::INFO,"Server.cpp", "Starting CGI execution.");
		std::string rs;
		std::string rs_start_line = "HTTP/1.1 200 OK\r\n";
		Logger::log(Logger::DEBUG,"Server.cpp", "Request details: method:  " + method + ", path: " + path + ", body size: " + to_string(body.size()) + "bytes");
		std::vector<Location>::iterator it;
		std::map<std::string, Location>::iterator it_map;
		//TODO: Verificar y hacer un algoritmo para manejar los distintos Locations y no ejecutar de mas findScriptPath
		/* Define a directory or a file from where the file should be searched (for example,
			if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is
			/tmp/www/pouic/toto/pouet).
		*/
		for (it = _locations.begin(); it != _locations.end(); it++) {
			std::cout << path << " " << it->get_path() << std::endl;
			if (starts_with(path, it->get_path())) {
				Logger::log(Logger::INFO,"Server.cpp", "Checking location: " + it->get_path());
				std::string path_tmp = it->findScriptPath(path);
				size_t dot_pos = path_tmp.rfind('.');
				if ((dot_pos != std::string::npos) && (dot_pos != path.length() - 1)) {
					Logger::log(Logger::INFO,"Server.cpp", "Matching script found: " + path_tmp);
					path = path_tmp;
					break ;
				}else if (it->get_auto_index()) {
					rs = generate_index_html(it->get_files(), path_tmp);
					rs_start_line.append(rs);
					client.send_response(rs_start_line);
					return ;
				}
				else {
					throw std::runtime_error("No script found for the given path");
				}
			}
		}


		//Capturar el id de la Cookie y resolver sus datos, para enviarlo al CGI
		std::string cookie_val = req.get_header_by_key("Cookie");
		Logger::log(Logger::INFO,"Server.cpp", "Verifing Cookie: " + cookie_val);
		Cookie cookie = handle_cookie_session(cookie_val);
		//Verifico si la Cookie es valida y lo dejo en env como HTTP_COOKIE="session=invalid/valid"

		std::string http_cookie = "HTTP_COOKIE=" + ("session=" + cookie.get_session() + "; session_id=" + cookie.get_session_id());
		char* env[] = {
			(char*)http_cookie.c_str(),
			NULL
		};
		Logger::log(Logger::INFO,"Server.cpp", "Executing script: " + path);
		//Gestiono la respuesta de la ejecucion del CGI
		rs = CGI(path, method, body, env).execute();
		// Agrego Set-cookie en caso de que lo requiera
		if (rs.find("Set-Cookie: session_id=") != std::string::npos){
			cookie.set_session("valid");
			if (!cookie.empty())
				_cookies.push_back(cookie);
		}
		rs_start_line.append(rs);
		Logger::log(Logger::INFO,"Server.cpp", "Sending response to client. Response size: " + to_string(rs_start_line.size()) + "bytes");
		client.send_response(rs_start_line);
	}
	catch (const std::exception& e)
	{
		Logger::log(Logger::WARN,"Server.cpp", "CGI execution failed");
		Logger::log(Logger::WARN, "Server.cpp", e.what());
		Logger::log(Logger::WARN, "Server.cpp", "Request details: path: " + path +", method: " + method + ", body size: " + to_string(body.size()) + "bytes");
		client.send_error(500, "Internal Server Error");
	}
}