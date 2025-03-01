

#include "Client.hpp"
#include "Location.hpp"
#include "CGI.hpp"
#include "Config.hpp"
#include "HttpException.hpp"
#include "SessionCookieManager.hpp"
#include "Cookie.hpp"
#include "ServerConfig.hpp"

/**
 * @brief Constructor de la clase `Client`.
 * 
 * Inicializa un cliente con el descriptor de archivo del socket proporcionado
 * y crea un objeto `Request` vacío.
 * 
 * @param socket_fd Descriptor de archivo del socket asociado al cliente.
 */
Client::Client(int socket_fd, std::time_t _last_request, size_t n_req) :_socket_fd(socket_fd),
	_last_request(_last_request), _n_request(n_req),_request(), _error(std::make_pair(0, "")) {}
/**
 * @brief Destructor de la clase `Client`.
 * 
 * Libera los recursos asociados con el cliente. Actualmente no realiza
 * ninguna acción explícita.
 */
Client::~Client() {}
/**
 * @brief Obtiene la solicitud (`Request`) del cliente.
 * 
 * Este método devuelve una copia del objeto `Request` asociado al cliente.
 * 
 * @return Una copia del objeto `Request` del cliente.
 */
Request Client::get_request() const {
	return _request;
}

/**
 * @brief Obtiene el descriptor de archivo del socket asociado al cliente.
 * 
 * Este método devuelve el descriptor de archivo del socket (`_socket_fd`) 
 * que representa la conexión del cliente con el servidor.
 * 
 * @return El descriptor de archivo del socket asociado al cliente.
 */
int Client::get_socket_fd() const {
	return _socket_fd;
}
void Client::reset_last_request() {
	_last_request = std::time(0);
}
/**
 * @brief Método para recibir datos desde un socket y procesar la solicitud recibida.
 * 
 * Este método escucha datos provenientes del socket asociado a la instancia. 
 * Si se reciben datos, estos se procesan a través del método `parse_request` de la clase `_request`.
 * Si no se reciben datos (por ejemplo, el cliente se desconecta), el socket se cierra.
 * 
 * @return true Si se reciben datos correctamente y se procesan.
 * @return false Si el cliente se desconecta o no se reciben datos.
 */
int Client::handle_request(ServerConfig srv_conf) {
	try {
		char buffer[1024];
		std::string request_data;
		int bytes_received;

		//_request.set_state(0);
		while (true) {
			bytes_received = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
			std::cout << "bytes_received " << bytes_received << std::endl; 
			if (bytes_received > 0) {
				Logger::log(Logger::INFO, "Client.cpp", "Parsing Request.");
				request_data.append(buffer, bytes_received);

			} else if (bytes_received == 0 ) {
				if (!request_data.empty())
					break;
				// Cliente desconectado
				Logger::log(Logger::WARN, "Client.cpp", "Cliente desconectado, socket fd" + to_string(_socket_fd));
				return -1;
			}
			else {
				std::cerr << to_string(errno) << " " <<strerror(errno) << std::endl;
				if (!request_data.empty())
					break;
				Logger::log(Logger::ERROR, "Client.cpp", "Error en lectura de socket fd" + to_string(_socket_fd));
				return -1;
			}
		}
		_request.parser(request_data);
		Location loc;
		if (!_request.get_path().empty() && _request.get_location().empty()) {
			loc = srv_conf.findMatchingLocation(_request.get_path());
			_request.set_location(loc);
		}
	} catch (HttpException::BadRequestException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		_error = std::make_pair<int, std::string>(400, "Bad Request");
	} catch (HttpException::NotFoundException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		_error = std::make_pair<int, std::string>(404, "Not Found");
	} catch (HttpException::RequestEntityTooLargeException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		_error = std::make_pair<int, std::string>(413, "Request Entity Too Large");
	} catch (HttpException::HTTPVersionNotSupportedException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		_error = std::make_pair<int, std::string>(505, "HTTP Version Not Supported");
	} catch (HttpException::RequestTimeoutException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		_error = std::make_pair<int, std::string>(408, "Request Timeout Exception");
	} catch (HttpException::RequestURITooLongException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		_error = std::make_pair<int, std::string>(414, "Request-URI Too Long");
	}
	
	return 0;
}


std::string resolve_html_path(std::string path) {
	std::string rs;
	if (path[0] == '/')
		path.erase(0,1);

	Logger::log(Logger::INFO,"Client.cpp", "resolve_html_path: " + path);
	rs = "Content-Type: text/html\r\n\r\n";
	std::ifstream	inFile(path.c_str());
	if (!inFile)
		throw HttpException::InternalServerErrorException();
	std::string line;
	while (std::getline(inFile, line))
		rs += line + "\n";
	inFile.close();
	return rs;
}

std::string create_start_line(int code, std::string message){
	std::string start_line = "HTTP/1.1 " + to_string(code) +" "+ message +"\r\n";
	return start_line;
}

void Client::handle_connection(const ServerConfig& srv_conf, std::string& rs_start_line) {
	std::string connection = _request.get_header_by_key("Connection");

	if (connection.empty()) {
		rs_start_line.append("Connection: close");
	} else if (srv_conf.get_timeout() > 0 && connection.compare("keep-alive") == 0 && !has_max_req(srv_conf.get_max_req())) {
		rs_start_line.append("Connection: Keep-Alive\r\n");
		rs_start_line.append("Keep-Alive: timeout=" + to_string(srv_conf.get_timeout()) + ", max=" + to_string(srv_conf.get_max_req()) + "\r\n");
	}
}


Cookie Client::handle_cookie() {
	std::string cookie_val = _request.get_header_by_key("Cookie");
	SessionCookieManager& sessionCM = SessionCookieManager::getInstance();
	Cookie cookie;

	if (!cookie_val.empty()) {
		std::size_t pos_session_id = cookie_val.find("session_id=");
		if (pos_session_id != std::string::npos && cookie_val.find(";", pos_session_id) != std::string::npos) {
			std::string session_id = extractStrBetween(cookie_val, "session_id=", ";");
			Logger::log(Logger::DEBUG, "Client.cpp", "Found Cookie: " + cookie_val + " Validating...");
			cookie = sessionCM.getCookieBySessionId(session_id);
		}
	}
	return cookie;
}

std::string Client::prepare_cgi_data(const ServerConfig& srv_conf, Cookie cookie) {
	_request.set_header("X-Forwarded-For", _ip);
	_request.set_header("X-Forwarded-Port", to_string(srv_conf.get_port()));

	std::string http_cookie;
	if (!cookie.isEmpty()) {
		std::string session_status = SessionCookieManager::getInstance().isCookieExpired(cookie) ? "expired" : "valid";
		http_cookie = "HTTP_COOKIE=session=" + session_status + "; session_id=" + cookie.value;
	}

	return http_cookie;
}

void Client::update_cookie_from_response(const std::string& response, Cookie& cookie) {
    SessionCookieManager& sessionCM = SessionCookieManager::getInstance();

    if (response.find("Set-Cookie: session_id=") != std::string::npos) {
        std::size_t pos_session_id = response.find("session_id=");
        if (cookie.isEmpty() && pos_session_id != std::string::npos && response.find(";", pos_session_id) != std::string::npos) {
            std::string session_id = extractStrBetween(response, "Set-Cookie: session_id=", ";");
            cookie = sessionCM.setCookieBySessionId(session_id, 300);
        }
    }
}

int Client::handle_response(ServerConfig  srv_conf) {
	std::string script_path;
	std::string rs;
	std::string rs_start_line = create_start_line(200, "OK");
	std::string path = _request.get_path();
	std::vector<std::string> files;
	Location loc = srv_conf.findMatchingLocation(path);
	std::string method = _request.get_method();
	Logger::log(Logger::DEBUG, "Client.cpp", "Location found: "+ loc.get_path());

	if (has_error()){
		rs_start_line = create_start_line(_error.first, _error.second);
		std::string path_error = srv_conf.get_error_page_by_code(204);
		rs = resolve_html_path(path_error);
		rs_start_line.append(rs);
		send_response(rs_start_line);
		return 0;
	}
	try {
		if (!loc.get_limit_except().isMethodAllowed(_request.get_method()))
			throw HttpException::NotAllowedMethodException();
		else if (!loc.get_redirect_url().empty())
			throw HttpException::MovedPermanentlyRedirectionException();
		if (loc.findScriptPath(path, script_path)) {
			if (loc.get_auto_index()) {
				files = loc.get_files();
				Logger::log(Logger::INFO,"Client.cpp", "Generating index: " + script_path);
				rs = generate_index_html(files, script_path);
				rs_start_line.append(rs);
				send_response(rs_start_line);
			}
			throw HttpException::NoContentException();
		}
		size_t dot_pos = script_path.rfind('.');
		if ((dot_pos != std::string::npos) && (dot_pos != path.length() - 1))
		{
			if (ends_with(script_path, ".html")) {
				if (method != "HEAD")
					resolve_html_path(script_path);
				else 
					throw HttpException::NoContentException();
			} else {
				//KEEP ALIVE
				handle_connection(srv_conf, rs_start_line);
				Cookie cookie = handle_cookie();
				std::string http_cookie = prepare_cgi_data(srv_conf, cookie);
				std::string root_dir = loc.get_root_directory();
				script_path = extractStrEnd(script_path, loc.get_root_directory());
				if (script_path[0] == '/')
					script_path.erase(0, 1);
				if (root_dir[0] == '/')
					root_dir.erase(0, 1);
				CGI cgi(root_dir, script_path, _request);
				cgi.resolve_cgi_env(_request, http_cookie);
				rs = cgi.execute();
				update_cookie_from_response(rs, cookie);
				//RESPONSE
				rs_start_line.append(rs);
				send_response(rs_start_line);
			}
		}
		return 0;
	}
	catch(HttpException::NotAllowedMethodException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		rs_start_line = create_start_line(405, "Not Allowed Method");
		std::string path_error = srv_conf.get_error_page_by_code(405);
		rs = resolve_html_path(path_error);
	}
	catch(HttpException::BadRequestException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		rs_start_line = create_start_line(400, "Bad Request");
		std::string path_error = srv_conf.get_error_page_by_code(400);
		rs = resolve_html_path(path_error);
		
	}
	catch(HttpException::NoContentException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		rs_start_line = create_start_line(204, "Not Content");
		std::string path_error = srv_conf.get_error_page_by_code(204);
		rs = resolve_html_path(path_error);
	}
	catch(HttpException::NotFoundException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		rs_start_line = create_start_line(404, "Not Found");
		std::string path_error = srv_conf.get_error_page_by_code(404);
		rs = resolve_html_path(path_error);
	}
	catch(HttpException::MovedPermanentlyRedirectionException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		rs_start_line = create_start_line(301, "Moved Permanently Redirection");
		rs = "Content-Type: text/html\r\n";
		rs.append("Location: "+ loc.get_redirect_url() +"\r\n\r\n");
		rs.append("<html><body>Redirigiendo...</body></html>");
	}
	catch(HttpException::RequestTimeoutException &e) {
		Logger::log(Logger::ERROR, "Client.cpp", e.what());
		rs_start_line = create_start_line(408, "Request Timeout");
		std::string path_error = srv_conf.get_error_page_by_code(408);
		rs = resolve_html_path(path_error);
	}
	rs_start_line.append(rs);
	send_response(rs_start_line);
	return 0;
}


void Client::send_response(std::string &response) {
	if (!response.empty()) {
		Logger::log(Logger::INFO, "Client.cpp", response.substr(0, response.find("\n")));
		send(_socket_fd, response.c_str(), response.size(), 0);
	}
}

void Client::send_error(int code, const std::string& message) {
	std::string response = "HTTP/1.1 " + to_string(code) + " " + message + "\r\n\r\n";
	send(_socket_fd, response.c_str(), response.size(), 0);
}

size_t  Client::has_client_timed_out() {
	return (std::time(0) - _last_request);
}

bool  Client::has_max_req(size_t n_req) {
	return  _n_request > n_req;
}

std::string Client::get_port() const {
	return _port;
}

std::string Client::get_ip() const{
	return _ip;
}

void Client::set_ip(std::string ip) {
	_ip = ip;
}

void Client::set_port(std::string port) {
	_ip = port;
}

bool Client::has_error() {
	return (_error.first != 0 && !_error.second.empty());
}