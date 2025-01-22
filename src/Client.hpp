#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <cstdlib>
#include "Request.hpp"
#include "Response.hpp"
class Client {
private:
    int _socket_fd;
	Request _request;

public:
    Client(int socket_fd) : _socket_fd(socket_fd), _request() {}
    ~Client() {}
	

    // Método para recibir datos
    bool receive_data() {
        char buffer[1024];
        int bytes_received = recv(_socket_fd, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::cout << "Parser Request "<< std::endl;
			_request.parse_request(buffer, bytes_received);
            return true;
        } else {
            // Cliente desconectado
            std::cout << "Cliente desconectado, socket fd: " << _socket_fd << "\n";
            close(_socket_fd);
            return false;
        }
    }
	Request get_request() const {
		return _request;
	}


    // Método para enviar la respuesta
    void send_response(std::string &response) {
        if (!response.empty()) {
            std::cout << response.substr(0, response.find("\n")) << std::endl;
            send(_socket_fd, response.c_str(), response.size(), 0);
		close(_socket_fd);
        }
	}
	void send_error(int code, const std::string& message) {
		std::string response = "HTTP/1.1 " + to_string(code) + " " + message + "\r\n\r\n";
		send(_socket_fd, response.c_str(), response.size(), 0);
		close(_socket_fd);
	}
    int get_socket_fd() const {
        return _socket_fd;
    }
};
