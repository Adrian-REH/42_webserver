
#include "Request.hpp"

const std::string CONTENT_LENGTH = "Content-Length";

/**
 * @brief Analiza la línea inicial de la solicitud.
 * 
 * @param start_line Línea inicial de la solicitud (método, ruta, protocolo).
 * @throws std::runtime_error Si la línea inicial está malformada.
 */
void Request::parse_start_line(const std::string& start_line) {
	std::deque<std::string> parts = split(start_line, ' ');
	if (parts.size() != 3) {
		throw std::runtime_error("Malformed request line: " + start_line);
	}
	_method = parts[0];
	_path = parts[1];
	_protocol = parts[2];
}
/**
 * @brief Analiza la sección de encabezados de la solicitud.
 * 
 * @param headers_section Cadena que contiene los encabezados.
 * @throws std::runtime_error Si falta algún encabezado obligatorio (por ejemplo, Host).
 */
void Request::parse_headers(const std::string& headers_section) {
	std::istringstream stream(headers_section);
	std::string line;
	//TODO: Error with parser header example: Cookie : session_id=14332Content-Type: text/html
	//Itero linea a linea buscando : para parsear key value to map
	while (std::getline(stream, line) && line != "\r") {
		size_t colon_pos = line.find(": ");
		if (colon_pos != std::string::npos) {
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 2);
			_headers[key] = value;
		}
	}
	//Si es importante algun header, como el HOST entonces doy error si no lo encuentro
	if (_headers.find("Host") == _headers.end()) {
		throw std::runtime_error("Missing Host header.");
	}
}
/**
 * @brief Analiza el cuerpo de la solicitud HTTP.
 * 
 * @param body_section Cadena que contiene el cuerpo de la solicitud.
 * @param content_length Tamaño esperado del cuerpo, según el encabezado Content-Length.
 * @throws std::runtime_error Si el tamaño del cuerpo no coincide con Content-Length.
 */
void Request::parse_body(const std::string& body_section, unsigned long content_length) {
	if (body_section.size() != (size_t)content_length) {
		throw std::runtime_error("Body size mismatch with Content-Length.");
	}
	_body = body_section;
}

Request::Request(): _method(""), _path(""), _protocol(""), _body("") {}

/**
 * @brief Analiza una solicitud HTTP a partir de un buffer.
 * 
 * Divide la solicitud HTTP en secciones como Start-Line (POST / HTTP/1.1), 
 * Header (Content-Length: 10), Body (name=adr).
 * 
 * @param req String que contiene la solicitud HTTP.
 * @throws std::runtime_error Si la solicitud está malformada o falta información requerida.
 */
void Request::parse_request(std::string req) {
	//size_t content_length = 0; TODO: usar
	//std::cout << "FULL REQUEST" << std::endl;
	//std::cout << req << std::endl;
	size_t header_end = req.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		throw std::runtime_error("Malformed request: missing header-body separator.");
	}
	std::string start_line = req.substr(0, req.find("\r\n"));
	std::string headers_section = req.substr(req.find("\r\n") + 2, header_end - req.find("\r\n") - 2);
	std::string body_section = req.substr(header_end + 4);

	parse_start_line(start_line);
	parse_headers(headers_section);
	if (_headers.find(CONTENT_LENGTH) != _headers.end()) {
		char *endp = NULL;
		unsigned long content_length = strtol(_headers[CONTENT_LENGTH].c_str(), &endp, 10);
		parse_body(body_section, content_length);
	}
}

std::string Request::get_path() const {
	return _path;
}

std::string Request::get_method() const {
	return _method;
}

std::string Request::get_protocol() const {
	return _protocol;
}

std::string Request::get_body() const {
	return _body;
}

std::string Request::get_header_by_key(const std::string &key) {
	return _headers[key];
}

void Request::display_header() {
	std::map<std::string, std::string>::iterator it;
	for (it = _headers.begin(); it != _headers.end(); it ++){
		Logger::log(Logger::DEBUG, "Request.cpp", it->first + " : " + it->second);
	}
}
