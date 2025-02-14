

#include "Client.hpp"


/**
 * @brief Constructor de la clase `Client`.
 * 
 * Inicializa un cliente con el descriptor de archivo del socket proporcionado
 * y crea un objeto `Request` vacío.
 * 
 * @param socket_fd Descriptor de archivo del socket asociado al cliente.
 */
Client::Client(int socket_fd, std::time_t wait_time, std::time_t _last_request) :_socket_fd(socket_fd), _wait_time(wait_time),
	_last_request(_last_request), _request() {}
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
int Client::receive_data() {
	char buffer[1024];
	std::string request_data;
	int bytes_received;

	_request.set_state(0);
	// TODO: further look into the return values of recv
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
	_request.handle_request(request_data);
	return 0;
}
/*if (bytes_received > 0) {
		Logger::log(Logger::INFO, "Client.cpp", "Parsing Request.");
		_request.parse_request(buffer, bytes_received);
		if (_request.get_header_by_key("Connection") == "close") {
			Logger::log(Logger::INFO, "Client.cpp", "El cliente pidio desconectarse, client_fd: " + to_string(_socket_fd));
			return 1;
		}
		reset_last_request();
		return 0;
	} else {
		// Cliente desconectado
		Logger::log(Logger::ERROR, "Client.cpp", "ERROR: Cliente desconectado, socket fd" + to_string(_socket_fd));
		return -1;
	}*/
/**
 * @brief Envía una respuesta al cliente a través del socket y cierra la conexión.
 * 
 * Este método toma una respuesta en forma de cadena, la envía al cliente utilizando
 * el socket asociado y luego cierra el socket. Si la respuesta no está vacía,
 * imprime en la consola la primera línea de la respuesta (hasta el primer salto de línea).
 * 
 * @param response Referencia a un objeto `std::string` que contiene la respuesta a enviar.
 */
void Client::send_response(std::string &response) {
	if (!response.empty()) {
		Logger::log(Logger::INFO, "Client.cpp", response.substr(0, response.find("\n")));
		send(_socket_fd, response.c_str(), response.size(), 0);
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
void Client::send_error(int code, const std::string& message) {
	std::string response = "HTTP/1.1 " + to_string(code) + " " + message + "\r\n\r\n";
	send(_socket_fd, response.c_str(), response.size(), 0);
}

bool Client::has_client_timed_out() {
	return (std::time(0) - _last_request) > _wait_time;
}

