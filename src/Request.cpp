
#include "Request.hpp"
#include "Location.hpp"

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
	if (parts.size() > 1) {
		size_t pos = parts[1].find('?');
		if (pos != std::string::npos)
			_query_string = parts[1].substr(pos+1);
	}
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
	//Itero linea a linea buscando : para parsear key value to map
	Logger::log(Logger::DEBUG, "Request.cpp", "Request Header:");
	while (std::getline(stream, line) && line != "\r") {
		size_t colon_pos = line.find(": ");
		if (colon_pos != std::string::npos) {
			std::string key = line.substr(0, colon_pos);
			std::string value = strtrim(line.substr(colon_pos + 2));
			_headers[key] = value;
			Logger::log(Logger::DEBUG, "Request.cpp", "Key:" + key + ", Value:" + value);
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
	std::cout << "body_section size: " << body_section.size() << std::endl;
	std::cout << "content_length : "<< content_length << std::endl;
	if (body_section.size() != (size_t)content_length) {
		throw std::runtime_error("Body size mismatch with Content-Length.");
	}
	//_body = body_section;
}

void Request::receiving_headers()
{
	
	size_t header_end = _raw_req.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		if (_state != RECEIVING_HEADERS) {
			_state = RECEIVING_HEADERS;
			Logger::log(Logger::INFO, "Request.cpp", "Change State to: RECEIVING_HEADERS");
		}
		return ;//throw std::runtime_error("Malformed request: missing header-body separator.");
	}
	_state = RECEIVING_BODY;
	std::cout << "header_end : '" << header_end << "' size: " << _raw_req.size() << std::endl;
	std::string start_line = _raw_req.substr(0, _raw_req.find("\r\n"));
	std::string headers_section = _raw_req.substr(_raw_req.find("\r\n") + 2, header_end - _raw_req.find("\r\n") - 2);
	parse_start_line(start_line);
	parse_headers(headers_section);

	Logger::log(Logger::INFO, "Request.cpp", "Change State to: RECEIVING_BODY");
	receiving_body(_raw_req.substr(header_end + 4));
}

void Request::receiving_body(std::string body_section) {
	_body += body_section;
	if (_method == "GET" || _method == "HEAD" )
	{
		_state = DONE;
		return ;
	}
	std::cout << "_body '" << _body << "'" << std::endl;
	if (_body.empty())
		return ;
	
	//std::cout << _body.substr(0, _body.find("\r\n")) << "" <<_body.substr(_body.find("\r\n") + 2).find("\r\n") << "' " << std::endl;
	//std::cout << "_headers '" <<_headers[CONTENT_LENGTH] << "'" <<std::endl;
	if (!_body.empty() && _headers.find(CONTENT_LENGTH) != _headers.end() && _state < DONE) {
		//TODO: use content length to verify if body size matches 
		std::cout << "holaaa" << std::endl;
		unsigned long content_length = to_dec_ulong(_headers[CONTENT_LENGTH]);
		parse_body(_body, content_length);
		Logger::log(Logger::INFO, "Request.cpp", "Change State to: DONE REQ");
		_state = DONE;
	} else if (_headers.find("Transfer-Encoding") != _headers.end() && _state < DONE && strtrim(_headers["Transfer-Encoding"]) == "chunked") {	
		read_chunked_body();
	} else {

	}
	/**
	 *  the server SHOULD respond with 400 (bad request) if it cannot
	 * determine the length of the message, or with 411 (length required) if
	 * it wishes to insist on receiving a valid Content-Length.
	 * 
	 *  Messages MUST NOT include both a Content-Length header field and a 
	 * non-identity transfer-coding. If the message does include a non-identity 
	 * transfer-coding, the Content-Length MUST be ignored.

	 * When a Content-Length is given in a message where a message-body is
	 * allowed, its field value MUST exactly match the number of OCTETs in
	 * the message-body. HTTP/1.1 user agents MUST notify the user when an
	 * invalid length is received and detected.
	 * 
	 * 
	 */
	 
}

void Request::read_chunked_body() {
	std::istringstream stream(_body);
	std::string line;
	unsigned long chunk_size;
	std::string size;
	bool is_size_line = true;

	while (std::getline(stream, line) && line != "\r") {
		if (is_size_line)
		{
			size = line.substr(0, line.find("\r"));
			if (size.empty())
			{
				std::cout << "size.empty" << std::endl;
				return ;
			}
			//std::cout << "size " << size << std::endl;
			// Read body chunk
			chunk_size = to_hex_ulong(size);
			std::getline(stream, line);
		}
		
		//std::cout << "chunk_size " << chunk_size << std::endl;

		if (chunk_size == 0 && line.substr(0, line.find("\r")).size() == chunk_size)
		{
			//ENDING OF BODY
			_state = DONE;
			Logger::log(Logger::INFO, "Request.cpp", "Change State to: DONE CHUNKED BODY");
			return ;
		} else if (line.substr(0, line.find("\r")).size() < chunk_size)
		{
			//KEEP READING BYTES
			is_size_line = false;
			chunk_size -= line.substr(0, line.find("\r")).size() - 2;
		} else if (line.substr(0, line.find("\r")).size() > chunk_size)
			throw std::runtime_error("Body chunk of wrong size.");
	}
}

Request::Request(): _raw_req(""), _method(""), _path(""), _protocol(""), _body(""),_query_string(""), _state(INIT) {}

/**
 * @brief Analiza una solicitud HTTP a partir de un buffer.
 * 
 * Divide la solicitud HTTP en secciones como Start-Line (POST / HTTP/1.1), 
 * Header (Content-Length: 10), Body (name=adr).
 * 
 * @param req String que contiene la solicitud HTTP.
 * @throws std::runtime_error Si la solicitud está malformada o falta información requerida.
 */
void Request::parser(std::string req) {
	//size_t content_length = 0; TODO: usar
	Logger::log(Logger::DEBUG, "Request.cpp", "Last state: " + to_string(_state));

	if (_state == INIT || _state == RECEIVING_HEADERS) {
		_raw_req += req;
		receiving_headers();
	} else if (_state == RECEIVING_BODY) {
		receiving_body(req);
	}
	//TODO: Controlar si está DONE?
}


void Request::set_state(int state) {
	_state = state;
}


void Request::set_header(std::string key, std::string value) {
	std::map<std::string, std::string> ::iterator it = _headers.find(key);
	if (it != _headers.end())
		return ;
	_headers[key] = value;
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

void Request::set_location(Location& loc) {
	_location = loc;
}

Location Request::get_location() const {
	return _location;
}

std::string Request::get_query_string()const {
	return _query_string;
}

int Request::get_state() const {
	return _state;
}

std::string Request::get_header_by_key(const std::string &key) {
	std::map<std::string, std::string>::iterator it = _headers.find(key);

	if (it != _headers.end())
		return it->second;
	else
		return "";
}

std::map<std::string, std::string> Request::get_headers() const {
	return _headers;
}

void Request::display_header() {
	std::map<std::string, std::string>::iterator it;
	for (it = _headers.begin(); it != _headers.end(); it ++){
		Logger::log(Logger::DEBUG, "Request.cpp", it->first + " : " + it->second);
	}
}
void Request::display() {
	std::map<std::string, std::string>::iterator it;
	display_header();
	Logger::log(Logger::DEBUG, "Request.cpp", _body);
}
