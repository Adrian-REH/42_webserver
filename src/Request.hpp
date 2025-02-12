#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include "utils/Utils.hpp"
#include "Logger.hpp"

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
		void parse_start_line(const std::string& start_line);
		/**
		 * @brief Analiza la sección de encabezados de la solicitud.
		 * 
		 * @param headers_section Cadena que contiene los encabezados.
		 * @throws std::runtime_error Si falta algún encabezado obligatorio (por ejemplo, Host).
		 */
		void parse_headers(const std::string& headers_section);
		/**
		 * @brief Analiza el cuerpo de la solicitud HTTP.
		 * 
		 * @param body_section Cadena que contiene el cuerpo de la solicitud.
		 * @param content_length Tamaño esperado del cuerpo, según el encabezado Content-Length.
		 * @throws std::runtime_error Si el tamaño del cuerpo no coincide con Content-Length.
		 */
		void parse_body(const std::string& body_section, unsigned long content_length);

	public:
		Request();

		/**
		 * @brief Analiza una solicitud HTTP a partir de un buffer.
		 * 
		 * Divide la solicitud HTTP en secciones como Start-Line (POST / HTTP/1.1), 
		 * Header (Content-Length: 10), Body (name=adr).
		 * 
		 * @param req String que contiene la solicitud HTTP.
		 * @throws std::runtime_error Si la solicitud está malformada o falta información requerida.
		 */
		void parse_request(std::string req);
		std::string get_path() const;
		std::string get_method() const;
		std::string get_protocol() const;
		std::string get_body() const;
		std::string get_header_by_key(const std::string &key);
		void display_header();
};
#endif