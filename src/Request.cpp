
#include "Request.hpp"
#include "Location.hpp"
//TODO: Agregar constantes a una clase
const std::string CONTENT_LENGTH = "Content-Length";
const std::string PROTOCOL = "HTTP/1.1";

/**
 * @brief Analiza la línea inicial de la solicitud.
 * 
 * @param start_line Línea inicial de la solicitud (método, ruta, protocolo).
 * @throws std::runtime_error Si la línea inicial está malformada.
 */
void Request::parse_start_line(const std::string& start_line) {
	std::deque<std::string> parts = split(start_line, ' ');
	if (parts.size() != 3) {
		throw HttpException::BadRequestException("Malformed request line: " + start_line); 
	}
	_method = parts[0];
	_path = parts[1];

	if (_path.size() > 1024)
		throw HttpException::RequestURITooLongException();
	
	size_t pos = _path.find('?');
	if (pos != std::string::npos) {
		_query_string = _path.substr(pos + 1);
		_path = _path.substr(0, pos);
	}
	
	_protocol = parts[2];
	if (_protocol != PROTOCOL)
		throw HttpException::HTTPVersionNotSupportedException();
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
	
	if (_headers.find("Host") == _headers.end()) {
		throw HttpException::BadRequestException("Missing Host header."); 
	}
}
/**
 * @brief Analiza el cuerpo de la solicitud HTTP.
 * 
 * @param body_section Cadena que contiene el cuerpo de la solicitud.
 * @param content_length Tamaño esperado del cuerpo, según el encabezado Content-Length.
 * @throws std::runtime_error Si el tamaño del cuerpo no coincide con Content-Length.
 */
void Request::parse_body(const std::string& body_section, size_t content_length) {
	if (content_length > _location.get_client_max_body_size() || body_section.size() > content_length){
		throw HttpException::RequestEntityTooLargeException();
	} else if (body_section.size() == content_length) {
		_state = DONE;
	} 
}


void Request::receiving_headers()
{
	size_t header_end = _raw_req.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		if (_state != RECEIVING_HEADERS) {
			_state = RECEIVING_HEADERS;
			Logger::log(Logger::DEBUG, "Request.cpp", "Change State to: RECEIVING_HEADERS");
		}
		return ;
	}
	_state = RECEIVING_BODY;
	std::string start_line = _raw_req.substr(0, _raw_req.find("\r\n"));
	std::string headers_section = _raw_req.substr(_raw_req.find("\r\n") + 2, header_end - _raw_req.find("\r\n") - 2);
	parse_start_line(start_line);
	parse_headers(headers_section);
	Logger::log(Logger::DEBUG, "Request.cpp", "Change State to: RECEIVING_BODY");
	receiving_body(_raw_req.substr(header_end + 4));
}

void Request::receiving_body(std::string body_section) {
	_body += body_section;

	if (_method == "GET" || _method == "HEAD")
	{
		if (!_body.empty())
			throw HttpException::BadRequestException("Method does not required a message body.");
		_state = DONE;
		return ;
	}

	if (_body.empty())
		return ;
	
	if (is_chunked_request() && _state < DONE) {	
		read_chunked_body();
	} else if (!_body.empty() && _headers.find(CONTENT_LENGTH) != _headers.end() && _state < DONE) {
		size_t content_length = (size_t) to_dec_ulong(_headers[CONTENT_LENGTH]);
		parse_body(_body, content_length);
		Logger::log(Logger::DEBUG, "Request.cpp", "Change State to: DONE REQ");
	} else {
		throw HttpException::BadRequestException("No Content-Length or Transfer-Encoding header present.");
	}
}

void Request::read_chunked_body(){
	std::istringstream stream(_body);
	std::string line;
	std::string size_line;
	size_t chunk_size;
	size_t full_size = 0;
	bool is_size_line = true;
	std::string decoded_body;
	Logger::log(Logger::INFO, "Request.cpp", "Reading chunked body");
	while (std::getline(stream, line)) {
		if (is_size_line)
		{
			size_line = line.substr(0, line.find("\r"));
			if (size_line.empty())
				throw HttpException::BadRequestException("Multipart request malformed");
			// Read body chunk
			if (size_line.find(";") == std::string::npos)
				chunk_size = to_hex_ulong(strtrim(size_line));
			else
				chunk_size = to_hex_ulong(size_line.substr(0, size_line.find(";")));
			full_size += chunk_size;
			if (full_size > _location.get_client_max_body_size())
				throw HttpException::RequestEntityTooLargeException();
			std::getline(stream, line);
		}
		
		if (chunk_size == 0)
		{
			// Reads entity-header 
			if (_headers.find("Trailer") != _headers.end())
			{
				size_t trailer_end = line.find("\r");
				while (line != "\r" && trailer_end != std::string::npos)
				{
					size_t colon_pos = line.find(": ");
					if (colon_pos != std::string::npos) {
						std::string key = line.substr(0, colon_pos);
						if (key == "Transfer-Encoding" || key == CONTENT_LENGTH || key =="Trailer")
							throw HttpException::BadRequestException();
						std::string value = strtrim(line.substr(colon_pos + 2));
						_headers[key] = value;
					}
					std::getline(stream, line);
				}
			}
			_headers[CONTENT_LENGTH] = full_size;
			_body = decoded_body;
			_state = DONE;
			Logger::log(Logger::DEBUG, "Request.cpp", "Change State to: DONE CHUNKED BODY");
			return ;
		} else if (line.substr(0, line.find("\r")).size() < chunk_size)
		{
			Logger::log(Logger::DEBUG, "Request.cpp", "KEEP READING BYTES");
			is_size_line = false;
			decoded_body += (line + "\n");
			chunk_size -= (line.size() + 1);
		} else if (line.substr(0, line.find("\r")).size() > chunk_size)
			throw HttpException::BadRequestException("Body chunk of wrong size.");
		else {
			is_size_line = true;
			decoded_body += line.substr(0, line.find("\r"));
		}
			
	}
	throw HttpException::BadRequestException("Body chunk of wrong size.");
}

bool Request::is_chunked_request() {
	return (_headers.find("Transfer-Encoding") != _headers.end() && _headers["Transfer-Encoding"] == "chunked");
}


Request::Request(): _raw_req(""), _method(""), _path(""), _protocol(""), _body(""), _query_string(""),
 _state(INIT), _body_timeout(60), _body_time(0), _header_time(0), _header_timeout(60) {}


bool isExpired(size_t expiration) {
	std::time_t currentTime = std::time(0);
	return difftime(currentTime, expiration) > 0;
}

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
	try {
		Logger::log(Logger::DEBUG, "Request.cpp", "Last state: " + to_string(_state));
		if (_state == INIT || _state == RECEIVING_HEADERS) {
			if (_header_time == 0)
				_header_time = std::time(0) + _header_timeout;
			_raw_req += req;
			receiving_headers();
			if (isExpired(_header_time))
				throw HttpException::RequestTimeoutException("Timeout header read");
		} else if (_state == RECEIVING_BODY) {
			if (_body_time == 0)
				_body_time = std::time(0) + _body_timeout;
			receiving_body(req);
			if (isExpired(_body_time))
				throw HttpException::RequestTimeoutException("Timeout body read");
		}
	} catch (std::exception & e) {
			_state = DONE;
			throw;
	}
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
