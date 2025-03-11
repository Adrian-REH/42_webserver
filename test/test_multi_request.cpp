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
         * @brief Constructor por defecto para inicializar una solicitud HTTP.
         * 
         * Este constructor inicializa los valores por defecto para los atributos 
         * de la clase `HttpRequest`. Establece la dirección IP del servidor como 
         * "127.0.0.1" (localhost), el puerto como 80, la ruta como "/",
         * el host como "localhost", el número de intentos de conexión como 3 
         * y el tiempo de espera entre intentos como 100 segundos.
         * 
         * @note Los valores predeterminados pueden ser modificados posteriormente 
         * mediante los métodos correspondientes.
         */
        HttpRequest() : _server_ip("127.0.0.1"), _server_port(80), _path(""), _host("localhost"), _n_intent(3), _time_intent(100) {}


       /**
         * @brief Construye la solicitud HTTP configurando las cabeceras y la dirección del servidor.
         * 
         * Esta función configura la cabecera "Host" con la dirección especificada, configura
         * la familia de direcciones para IPv4, y prepara la dirección del servidor con el puerto 
         * y la dirección IP proporcionados. También se asegura de que la dirección IP sea válida.
         * 
         * @return HttpRequest& Referencia al objeto HttpRequest actualizado.
         */
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

        /**
         * @brief Cambia el numero de intentos de conexion en caso de error.
         * @param intent Numero de intentos de conexion (por defecto: 3).
         */
        HttpRequest &setIntent(const int &intent) {
            _n_intent = intent;
            return *this; // Devuelve una referencia al objeto actual
        }

        /**
         * @brief Establece la dirección IP del host desde donde se envía la solicitud.
         * 
         * Este método permite cambiar la dirección IP del host (por defecto: localhost) que se enviará en la solicitud HTTP.
         * 
         * @param host Dirección IP o nombre del host (por ejemplo, "localhost").
         * @return HttpRequest& Referencia al objeto actual para permitir la encadenación de llamadas.
         */
        HttpRequest &setHost(const std::string &host) {
            _host = host;
            return *this; // Devuelve una referencia al objeto actual
        }

        /**
         * @brief Establece la dirección IP del servidor al que se va a conectar.
         * 
         * Este método permite configurar la dirección IP del servidor al que se enviará la solicitud.
         * 
         * @param ip Dirección IP del servidor (por ejemplo, "192.168.1.1").
         * @return HttpRequest& Referencia al objeto actual para permitir la encadenación de llamadas.
         */
        HttpRequest &setServerIp(const std::string &ip) {
            _server_ip = ip;
            return *this; // Devuelve una referencia al objeto actual
        }

        /**
         * @brief Establece el puerto del servidor al que se va a conectar.
         * 
         * Este método permite configurar el puerto del servidor. 
         * 
         * @param port Puerto del servidor (por ejemplo, 80 para HTTP).
         * @return HttpRequest& Referencia al objeto actual para permitir la encadenación de llamadas.
         */
        HttpRequest &setServerPort(int port) {
            _server_port = port;
            return *this;
        }

        /**
         * @brief Establece la ruta del recurso en el servidor.
         * 
         * Este método permite configurar la ruta del recurso que se va a solicitar al servidor. 
         * 
         * @param path Ruta del recurso en el servidor (por ejemplo, "/index.html").
         * @return HttpRequest& Referencia al objeto actual para permitir la encadenación de llamadas.
         */
        HttpRequest &setPath(const std::string &path) {
            _path = path;
            return *this;
        }

        /**
         * @brief Añade una cabecera a la solicitud HTTP.
         * 
         * Este método permite añadir una nueva cabecera HTTP, clave-valor, a la solicitud.
         * 
         * @param key Clave de la cabecera (por ejemplo, "Content-Type").
         * @param value Valor de la cabecera (por ejemplo, "application/json").
         * @return HttpRequest& Referencia al objeto actual para permitir la encadenación de llamadas.
         */
        HttpRequest &addHeader(const std::string &key, const std::string &value) {
            _headers[key] = value;
            return *this;
        }

        /**
         * @brief Convierte las cabeceras en una cadena de texto para ser añadida a la solicitud HTTP.
         * 
         * Este método recorre el mapa de cabeceras y genera una cadena de texto con el formato adecuado para ser añadido a la solicitud.
         * 
         * @return std::string Cadena de texto con las cabeceras en el formato adecuado.
         */
        std::string headerToStr(){
            std::string result;
            std::map<std::string, std::string>::iterator it;

            for (it = _headers.begin(); it != _headers.end(); ++it) {
                result += it->first;
                result += ": ";
                result += it->second;
                result += "\r\n";
            }
            return result;
        }

        /**
         * @brief Intenta conectar con el servidor.
         * 
         * Este método realiza un intento de conexión con el servidor. Si la conexión falla, se reintenta varias veces según la configuración del número de intentos y el tiempo de espera entre cada intento.
         * 
         * @throws std::runtime_error Si no se puede conectar después de varios intentos.
         */
        void connectToServer() {
            //Solicitud de conexion al servidor
            if (connect(_sock, (struct sockaddr *)&_server_address, sizeof(_server_address)) < 0) {
                std::cerr << "Error al conectar al servidor, reintentando..." << std::endl;
                int intent = 0;
                while (intent < _n_intent) {
                    // Intentar conectar al servidor
                    sleep(_time_intent); // Esperar antes de reintentar
                    if (connect(_sock, (struct sockaddr *)&_server_address, sizeof(_server_address)) < 0) {
                        close(_sock);
                        intent++;
                    } else
                        break;
                }
                // No se pudo conectar al servidor
                if (intent >= _n_intent) {
                    close(_sock);
                    throw std::runtime_error("No se pudo conectar al servidor después de varios intentos.");
                }
            }
        }

        /**
         * @brief Envía una solicitud HTTP al servidor.
         * 
         * Este método construye la solicitud HTTP, incluyendo la cabecera y el cuerpo de la solicitud, y la envía al servidor.
         * 
         * @param method Tipo de solicitud HTTP (por ejemplo, "POST" o "GET").
         * @param path Ruta del recurso en el servidor.
         * @param body Cuerpo de la solicitud (por ejemplo, datos JSON).
         */
        void sendRequest(std::string method, std::string path, std::string body) {
            // Construir la solicitud HTTP
            std::string http_request = method + " " + _path + path + " HTTP/1.1\r\n";

            // Agregar Content-Length a la cabecera
            std::ostringstream content_length_stream;
            content_length_stream << body.size();  // Convertir el tamaño del cuerpo a string
            std::string content_length_str = content_length_stream.str();  // Obtener la cadena
            _headers["Content-Length"] = content_length_str;

            // Añadir las cabeceras a la solicitud
            http_request += headerToStr();

            // Añadir el cuerpo de la solicitud
            http_request += body;  // Cuerpo de la solicitud

            // Enviar la solicitud HTTP al servidor
            send(_sock, http_request.c_str(), http_request.size(), 0);
            std::cout << "--------------------\n" 
                << "HTTP-Request:"
                << std::endl
                << http_request
                << std::endl
                << "--------------------"
                << std::endl;
        }

        /**
         * @brief Lee la respuesta del servidor.
         * 
         * Este método lee la respuesta del servidor y la muestra en consola.
         * 
         * @note Si no se recibe respuesta, se muestra un mensaje de error.
         */
        void readResponse() {
            // Leer la respuesta del servidor
            std::string response;
            ssize_t bytes_received = read(_sock, _buffer, sizeof(_buffer) - 1);
            if (bytes_received > 0) {
                _buffer[bytes_received] = '\0'; // Asegurarse de que sea un string válido
                // Construir la respuesta
                response.append(_buffer, bytes_received);
                std::cout << "Response: " << response.substr(0, response.find("\n")) << std::endl;
            } else {
                std::cerr << "No se recibió respuesta del servidor." << std::endl;
            }
        }

        /**
         * @brief Realiza una solicitud HTTP POST al servidor.
         * 
         * Este método realiza una solicitud POST al servidor, enviando el cuerpo de la solicitud.
         * 
         * @param path Ruta del recurso en el servidor.
         * @param body Cuerpo de la solicitud (por ejemplo, datos JSON).
         * @return Response La respuesta del servidor.
         */
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

        /**
         * @brief Realiza una solicitud HTTP GET al servidor.
         * 
         * Este método realiza una solicitud GET al servidor.
         * 
         * @param path Ruta del recurso en el servidor.
         * @return Response La respuesta del servidor.
         */
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

        /**
         * @brief Realiza una solicitud HTTP DELETE al servidor.
         * 
         * Este método realiza una solicitud DELETE al servidor.
         * 
         * @param uri URI del recurso a eliminar en el servidor.
         * @return Response La respuesta del servidor.
         */
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

    req.post("/cgi-bin/", "nombre=adr&edad=10");

    req.get("/cgi-bin/");
    return 0;
}
