#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>  // Necesario para usar ostringstream
#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <map>
#include <stdlib.h>

class Response {
    private:
        std::string _body;
        std::string _status_code;
        std::string _header;
};

class HttpRequest {
    private:
        std::string _server_ip;
        int _server_port;
        std::string _host;
        std::map<std::string, std::string> _headers;
        std::string _path;
        struct sockaddr_in _server_address;
        int _sock;
        int _n_intent;
        int _time_intent;
        char _buffer[1024];
        Response _response;
    public:
        /**
         * @brief Constructor con parámetros opcionales y valores predeterminados.
         * @param server_ip Dirección IP del servidor (por defecto: "127.0.0.1").
         * @param host Dirección IP propia (por defecto: "localhost").
         * @param server_port Puerto del servidor (por defecto: 80).
         * @param url URL de la solicitud (por defecto: "").
         */
        HttpRequest() : _server_ip("127.0.0.1"), _server_port(80), _path("/"), _host("localhost"), _n_intent(3), _time_intent(100) {}

        HttpRequest build(){
            _headers["Host"] = _host;
            _server_address.sin_family = AF_INET;
            _server_address.sin_port = htons(_server_port);

            // Convertir la dirección IP a formato binario
            if (inet_pton(AF_INET, _server_ip.c_str(), &_server_address.sin_addr) <= 0) {
                std::cerr << "Dirección no válida o no soportada, reintentando..." << std::endl;
                sleep(100);
            }

    
            return *this;
        }


        HttpRequest &setIntent(const int &intent) {
            _n_intent = intent;
            return *this; // Devuelve una referencia al objeto actual
        }

        HttpRequest &setHost(const std::string &host) {
            _host = host;
            return *this; // Devuelve una referencia al objeto actual
        }
        HttpRequest &setServerIp(const std::string &ip) {
            _server_ip = ip;
            return *this; // Devuelve una referencia al objeto actual
        }

        HttpRequest &setServerPort(int port) {
            _server_port = port;
            return *this;
        }

        HttpRequest &setPath(const std::string &path) {
            _path = path;
            return *this;
        }

        HttpRequest &addHeader(const std::string &key, const std::string &value) {
            _headers[key] = value;
            return *this;
        }
        std::string headerToStr(){
            std::string result;
            std::map<std::string, std::string>::iterator it;

            for (it = _headers.begin(); it != _headers.end(); ++it) {
                result += it->first ;
                result += ": ";
                result += it->second;
                result += "\r\n";
            }
            return result;
        }

        void connectToServer() {
            //Solicitud de conexion al servidor
            if (connect(_sock, (struct sockaddr *)&_server_address, sizeof(_server_address)) < 0) {
                std::cerr << "Error al conectar al servidor, reintentando..." << std::endl;
                int intent = 0;
                while ( intent < _n_intent ) {
                    // Intentar conectar al servidor
                    sleep(_time_intent); // Esperar antes de reintentar
                    if (connect(_sock, (struct sockaddr *)&_server_address, sizeof(_server_address)) < 0) {
                        close(_sock);
                        intent++;
                    } else
                        break;
                }
                //No se pudo conectar al servidor
                if (intent >= _n_intent){
                    close(_sock);
                        throw std::runtime_error("No se pudo conectar al servidor después de varios intentos.");
                }
            }
        }



        void sendRequest(std::string method, std::string path,std::string body) {
            //Build Request
            //Type or request
            std::string http_request = method + " " + _path + path + " HTTP/1.1\r\n";

            //added Content-Length to Header
            std::ostringstream content_length_stream;
            content_length_stream << body.size();  // Convertir el tamaño del cuerpo a string
            std::string content_length_str = content_length_stream.str();  // Obtener la cadena
            _headers["Content-Length"] = content_length_str;
            //Added Header to Request
            http_request += headerToStr();
            //Added Body to request
            http_request += body;  // Cuerpo de la solicitud

            // Send HTTP Request to server
            send(_sock, http_request.c_str(), http_request.size(), 0);
            std::cout <<"--------------------\n" << "HTTP-Request:" << std::endl << http_request << std::endl <<"--------------------" << std::endl;

        }

        void readResponse() {
            // Leer la respuesta del servidor
            std::string response;
            ssize_t bytes_received = read(_sock, _buffer, sizeof(_buffer) - 1);
            if (bytes_received > 0) {
                _buffer[bytes_received] = '\0'; // Asegurarse de que sea un string válido
                //Build response
                response.append(_buffer, bytes_received);
				std::cout << "Response: " << response.substr(0, response.find("\n")) << std::endl;
            } else {
                std::cerr << "No se recibió respuesta del servidor." << std::endl;
            }
        }

        Response post(std::string path, std::string body) {
            // Crear el socket
            if ((_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                std::cerr << "Error al crear el socket, reintentando..." << std::endl;
                sleep(100);
            }
            connectToServer();
            sendRequest("POST", path, body);
            readResponse();
            close(_sock);
            return _response;
        }
        Response get(std::string path) {
            // Crear el socket
            if ((_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                std::cerr << "Error al crear el socket, reintentando..." << std::endl;
                sleep(100);
            }
            connectToServer();
            sendRequest("GET", path, "");
            readResponse();
            close(_sock);
            return _response;
        }
        Response del(std::string uri) {
            return _response;
        }
};

int main() {
    HttpRequest req = HttpRequest()
    .addHeader("Content-Type", "application/x-www-form-urlencoded")
    .setServerIp("127.0.0.1")
    .setServerPort(8080)
    .build();

    req.post("cgi-bin/app.py", "nombre=adr&edad=10");

    req.get("cgi-bin/app.py");
    return 0;
}
