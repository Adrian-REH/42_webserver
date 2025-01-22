#ifndef REQUEST_HTPP
#define REQUEST_HTPP
#include <string>
#include <map>
#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>

std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;  // Convertir el número en cadena
    return oss.str();  // Devolver el resultado como std::string
}
std::deque<std::string> split(std::string &str, char delimiter) {
	std::stringstream		sstr(str);
	std::string				token;
	std::deque<std::string>	result;
	while (std::getline(sstr, token, delimiter)){
		result.push_back(token);
	}
	return result;
}

class Request {
	private:
		std::string _method;
		std::string _path;
		std::string _protocol;
		std::string _body;
		std::map<std::string, std::string> _headers;
	public:
		Request(): _method(""), _path(""), _protocol(""), _body("") {}

		void parse_request(char *buffer, int byte) {
			std::string req;
			size_t cl_pos;
			std::deque<std::string> request_line;
			size_t content_length = 0;
			if (buffer && byte > 0)
				req.append(buffer, byte);

			//Obtener Method y Path
			request_line = split(req, ' ');
			if (request_line.size() > 0)
			{
				_method = request_line[0];
				_path = request_line[1];
				_protocol = request_line[2];
			}
			// Obtener el body y header Content-Length
			cl_pos = req.find("Content-Length: ");
			if (cl_pos != std::string::npos) {
					// Buscar el final de la línea después de "Content-Length: "
				size_t end_pos = req.find("\r\n", cl_pos);
				if (end_pos != std::string::npos) {
					// Extraer el valor numérico de Content-Length
					std::string content_length_str = req.substr(cl_pos + 16, end_pos - (cl_pos + 16));
					char* endp = NULL;
					unsigned long content_length = strtoul(content_length_str.c_str(), &endp, 10);

					// Validar que la conversión sea exitosa
					if (endp == content_length_str.c_str() + content_length_str.size()) {
						_headers["Content-Length"] = content_length_str; // Guardar en los headers
						_body = req.substr(req.size() - content_length);
					} else {
						// Manejar error: valor inválido en Content-Length
						throw std::runtime_error("Invalid Content-Length value.");
					}
				} else {
					// Manejar error: línea mal formada
					throw std::runtime_error("Malformed request: missing end of line for Content-Length.");
				}
			}
			// Obtener el header Content-Type
			cl_pos = req.find("Content-Type: ");
			if (cl_pos != std::string::npos) {
					// Buscar el final de la línea después de "Content-Type: "
				size_t end_pos = req.find("\r\n", cl_pos);
				if (end_pos != std::string::npos) {
					// Extraer el valor str de Content-Type
					std::string content_type_str = req.substr(cl_pos + 16, end_pos - (cl_pos + 16));
					_headers["Content-Type"] = content_type_str; // Guardar en los headers

				} else {
					// Manejar error: línea mal formada
					throw std::runtime_error("Malformed request: missing end of line for Content-Type.");
				}
			}

			// Obtener el Host
			cl_pos = req.find("Host: ");
			if (cl_pos != std::string::npos) {
					// Buscar el final de la línea después de "Host: "
				size_t end_pos = req.find("\r\n", cl_pos);
				if (end_pos != std::string::npos) {
					// Extraer el valor str de Host
					std::string content_type_str = req.substr(cl_pos + 16, end_pos - (cl_pos + 16));
					_headers["Host"] = content_type_str; // Guardar en los headers

				} else {
					// Manejar error: línea mal formada
					throw std::runtime_error("Malformed request: missing end of line for Host.");
				}
			} else {
				// Manejar error: Host no encontrado
				throw std::runtime_error("Host header not found.");
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