#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>  // Necesario para usar ostringstream

int main() {
    const char *server_ip = "127.0.0.1"; // Dirección localhost
    int server_port = 8080;             // Puerto del servidor
    int sock = 0;
    struct sockaddr_in server_address;
    char buffer[1024] = {0};
    const std::string body = "nombre=adr&edad=10";
    std::ostringstream content_length_stream;
    content_length_stream << body.size();  // Convertir el tamaño del cuerpo a string
    std::string content_length_str = content_length_stream.str();  // Obtener la cadena

    std::string http_request = "POST /cgi-bin/app.py HTTP/1.1\r\n";
    http_request += "Host: localhost\r\n";
    http_request += "Content-Type: application/x-www-form-urlencoded\r\n";  // Tipo de contenido
    http_request += "Content-Length: "+ content_length_str  + "\r\n";  // Longitud del cuerpo
    http_request += "\r\n";  // Línea en blanco separando los encabezados del cuerpo
    http_request += body;  // Cuerpo de la solicitud

    for (int i = 0; i < 5; i++) {


        std::cout << "Intentando conectar al servidor en " << server_ip << ":" << server_port << "...\n";

        // Crear el socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error al crear el socket, reintentando..." << std::endl;
            sleep(100);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server_port);

        // Convertir la dirección IP a formato binario
        if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
            std::cerr << "Dirección no válida o no soportada, reintentando..." << std::endl;
            close(sock);
            sleep(100);
        }

        // Intentar conectar al servidor
        if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            std::cerr << "Error al conectar al servidor, reintentando..." << std::endl;
            close(sock);
            sleep(100); // Esperar antes de reintentar
        }

        std::cout << "Conexión exitosa.\n";

        // Enviar solicitud HTTP al servidor
        send(sock, http_request.c_str(), http_request.size(), 0);
        std::cout << "Solicitud enviada:\n" << http_request << std::endl;

        // Leer la respuesta del servidor
        ssize_t bytes_received = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Asegurarse de que sea un string válido
            std::cout << "Respuesta del servidor:\n" << buffer << std::endl;
        } else {
            std::cerr << "No se recibió respuesta del servidor." << std::endl;
        }

        // Cerrar el socket
       // close(sock);

        // Salir del bucle después de una conexión exitosa
       // break;
    }
    return 0;
}


int main2() {
    const char *server_ip = "127.0.0.1"; // Dirección localhost
    int server_port = 8080;             // Puerto del servidor
    int sock = 0;
    struct sockaddr_in server_address;
    char buffer[1024] = {0};
    const std::string body = "nombre=adr&edad=10";
    std::ostringstream content_length_stream;
    content_length_stream << body.size();  // Convertir el tamaño del cuerpo a string
    std::string content_length_str = content_length_stream.str();  // Obtener la cadena

    std::string http_request = "POST / HTTP/1.1\r\n";
    http_request += "Host: localhost\r\n";
    http_request += "Content-Type: application/x-www-form-urlencoded\r\n";  // Tipo de contenido
    http_request += "Content-Length: "+ content_length_str  + "\r\n";  // Longitud del cuerpo
    http_request += "\r\n";  // Línea en blanco separando los encabezados del cuerpo
    http_request += body;  // Cuerpo de la solicitud

        std::cout << "Intentando conectar al servidor en " << server_ip << ":" << server_port << "...\n";

        // Crear el socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error al crear el socket, reintentando..." << std::endl;
            sleep(100);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server_port);

        // Convertir la dirección IP a formato binario
        if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
            std::cerr << "Dirección no válida o no soportada, reintentando..." << std::endl;
            close(sock);
            sleep(100);
        }

        // Intentar conectar al servidor
        if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            std::cerr << "Error al conectar al servidor, reintentando..." << std::endl;
            close(sock);
            sleep(100); // Esperar antes de reintentar
        }

        std::cout << "Conexión exitosa.\n";

        // Enviar solicitud HTTP al servidor
        send(sock, http_request.c_str(), strlen(http_request.c_str()), 0);
        std::cout << "Solicitud enviada:\n" << http_request << std::endl;

        // Leer la respuesta del servidor
        ssize_t bytes_received = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Asegurarse de que sea un string válido
            std::cout << "Respuesta del servidor:\n" << buffer << std::endl;
        } else {
            std::cerr << "No se recibió respuesta del servidor." << std::endl;
        }

        // Cerrar el socket
       // close(sock);

        // Salir del bucle después de una conexión exitosa
       // break;

    return 0;
}
