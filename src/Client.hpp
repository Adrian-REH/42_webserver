#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include "utils/Utils.hpp"
#include "Request.hpp"

/**
 * @brief Clase que representa un cliente conectado al servidor.
 * 
 * La clase `Client` encapsula la información y operaciones relacionadas con un cliente,
 * como el socket asociado y la solicitud (`Request`) enviada por el cliente.
 */
class Client {
private:
	int _socket_fd;
	Request _request;

public:
	/**
	 * @brief Constructor de la clase `Client`.
	 * 
	 * Inicializa un cliente con el descriptor de archivo del socket proporcionado
	 * y crea un objeto `Request` vacío.
	 * 
	 * @param socket_fd Descriptor de archivo del socket asociado al cliente.
	 */
	Client(int socket_fd) : _socket_fd(socket_fd), _request() {}
	/**
	 * @brief Destructor de la clase `Client`.
	 * 
	 * Libera los recursos asociados con el cliente. Actualmente no realiza
	 * ninguna acción explícita.
	 */
	~Client() {}
	/**
	 * @brief Obtiene la solicitud (`Request`) del cliente.
	 * 
	 * Este método devuelve una copia del objeto `Request` asociado al cliente.
	 * 
	 * @return Una copia del objeto `Request` del cliente.
	 */
	Request get_request() const {
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
	int get_socket_fd() const {
		return _socket_fd;
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
	int receive_data() {
		char buffer[1024];
		int bytes_received = recv(_socket_fd, buffer, sizeof(buffer), 0);
		if (bytes_received > 0) {
			Logger::log(Logger::INFO, "Client.cpp", "Parsing Request.");
			_request.parse_request(buffer, bytes_received);
			return 0;
		} else {
			// Cliente desconectado
			Logger::log(Logger::ERROR, "Client.cpp", "Cliente desconectado, socket fd" + to_string(_socket_fd));
			return -1;
		}
	}
	/**
	 * @brief Envía una respuesta al cliente a través del socket y cierra la conexión.
	 * 
	 * Este método toma una respuesta en forma de cadena, la envía al cliente utilizando
	 * el socket asociado y luego cierra el socket. Si la respuesta no está vacía,
	 * imprime en la consola la primera línea de la respuesta (hasta el primer salto de línea).
	 * 
	 * @param response Referencia a un objeto `std::string` que contiene la respuesta a enviar.
	 */
	void send_response(std::string &response) {
		if (!response.empty()) {
			Logger::log(Logger::INFO, "Client.cpp", response.substr(0, response.find("\n")));
			send(_socket_fd, response.c_str(), response.size(), 0);
		close(_socket_fd);
		}
	}
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
	void send_error(int code, const std::string& message) {
		std::string response = "HTTP/1.1 " + to_string(code) + " " + message + "\r\n\r\n";
		send(_socket_fd, response.c_str(), response.size(), 0);
		close(_socket_fd);
	}
};

#endif