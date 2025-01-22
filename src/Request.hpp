#ifndef REQUEST_HTPP
#define REQUEST_HTPP
#include <string>
#include <map>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils/split.hpp"

const std::string CONTENT_LENGTH = "Content-Length";

/**
 * @brief Convierte un valor entero a una cadena de texto.
 * 
 * @param value El valor entero a convertir.
 * @return std::string Representación en cadena del entero.
 */
std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


/**
 * @class Request
 * @brief Representa una solicitud HTTP con funcionalidad para analizarla.
 * 
 * Esta clase permite analizar y obtener información de una solicitud HTTP,
 * incluyendo método, ruta, protocolo, encabezados y cuerpo.
 */
class Request {
	private:
		std::string _method;
		std::string _path;
		std::string _protocol;
		std::string _body;
		std::map<std::string, std::string> _headers;
		/**
		 * @brief Analiza la línea inicial de la solicitud.
		 * 
		 * @param start_line Línea inicial de la solicitud (método, ruta, protocolo).
		 * @throws std::runtime_error Si la línea inicial está malformada.
		 */
		void parse_start_line(const std::string& start_line) {
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
		void parse_headers(const std::string& headers_section) {
			std::istringstream stream(headers_section);
			std::string line;
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
		void parse_body(const std::string& body_section, unsigned long content_length) {
			if (body_section.size() != (size_t)content_length) {
				throw std::runtime_error("Body size mismatch with Content-Length.");
			}
			_body = body_section;
		}
	public:
		Request(): _method(""), _path(""), _protocol(""), _body("") {}

		/**
		 * @brief Analiza una solicitud HTTP a partir de un buffer.
		 * 
		 * Divide la solicitud HTTP en secciones como Start-Line (POST / HTTP/1.1), 
		 * Header (Content-Length: 10), Body (name=adr).
		 * 
		 * @param buffer Puntero al buffer que contiene la solicitud HTTP.
		 * @param byte Tamaño en bytes del buffer.
		 * @throws std::runtime_error Si la solicitud está malformada o falta información requerida.
		 */
		void parse_request(char *buffer, int byte) {
			std::string req;
			size_t cl_pos;
			size_t content_length = 0;
			if (buffer && byte > 0)
				req.append(buffer, byte);

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
		
		std::string get_path() const {
			return _path;
		}
		std::string get_method() const {
			return _method;
		}
		std::string get_protocol() const {
			return _protocol;
		}
		std::string get_body() const {
			return _body;
		}
};
#endif