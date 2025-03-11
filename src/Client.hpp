#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "utils/Utils.hpp"
#include "Request.hpp"
#include "Logger.hpp"
#include <ctime>
#include "SessionCookieManager.hpp"
#include "Cookie.hpp"
#include "ServerConfig.hpp"
/**
 * @brief Clase que representa un cliente conectado al servidor.
 * 
 * La clase `Client` encapsula la información y operaciones relacionadas con un cliente,
 * como el socket asociado y la solicitud (`Request`) enviada por el cliente.
 */
class Client {
	private:
		int _socket_fd;
		std::string _ip;
		std::string _port;
		size_t _last_request;
		size_t _n_request;
		Request _request;
		std::pair<int, std::string> _error;
		void handle_connection(const ServerConfig& srv_conf, std::string& rs_start_line);
		Cookie handle_cookie();
		std::string prepare_cgi_data( const ServerConfig &srv_conf, Cookie cookie);
		void update_cookie_from_response(const std::string& response, Cookie& cookie);

	public:

		/**
		 * @brief Constructor de la clase `Client`.
		 * 
		 * Inicializa un cliente con el descriptor de archivo del socket proporcionado
		 * y crea un objeto `Request` vacío.
		 * 
		 * @param socket_fd Descriptor de archivo del socket asociado al cliente.
		 */
		Client(int socket_fd, std::time_t _last_request = std::time(0), size_t n_req = 0);
		/**
		 * @brief Destructor de la clase `Client`.
		 * 
		 * Libera los recursos asociados con el cliente. Actualmente no realiza
		 * ninguna acción explícita.
		 */
		~Client();

		/**
		 * @brief Obtiene la solicitud (`Request`) del cliente.
		 * 
		 * Este método devuelve una copia del objeto `Request` asociado al cliente.
		 * 
		 * @return Una copia del objeto `Request` del cliente.
		 */
		int handle_response(ServerConfig srv_conf);

		/**
		 * @brief Obtiene el descriptor de archivo del socket asociado al cliente.
		 * 
		 * Este método devuelve el descriptor de archivo del socket (`_socket_fd`) 
		 * que representa la conexión del cliente con el servidor.
		 * 
		 * @return El descriptor de archivo del socket asociado al cliente.
		 */
		int get_socket_fd() const;
		bool has_error();
		void reset_last_request();
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
		int handle_request(ServerConfig);
		/**
		 * @brief Envía una respuesta al cliente a través del socket y cierra la conexión.
		 * 
		 * Este método toma una respuesta en forma de cadena, la envía al cliente utilizando
		 * el socket asociado y luego cierra el socket. Si la respuesta no está vacía,
		 * imprime en la consola la primera línea de la respuesta (hasta el primer salto de línea).
		 * 
		 * @param response Referencia a un objeto `std::string` que contiene la respuesta a enviar.
		 */
		void send_response(std::string &response);
		/**
		 * @brief Envía un mensaje de error HTTP al cliente y cierra la conexión.
		 * 
		 * Este método construye una respuesta HTTP de error con el código de estado proporcionado
		 * y el mensaje correspondiente. Luego, envía la respuesta al cliente a través del socket
		 * y cierra la conexión con el cliente.
		 * 
		 * @param code Código de error HTTP que indica el tipo de error (por ejemplo, 404, 500).
		 * @param message Mensaje de error que describe el motivo del error (por ejemplo, "Not Found", "Internal Server Error").
		 */
		void send_error(int code, const std::string& message);
		size_t  has_client_timed_out();
		bool  has_max_req(size_t n_req);
		std::string get_port() const;
		std::string get_ip() const;
		Request get_request() const;

		void set_ip(std::string);
		void set_port(std::string);
};

#endif