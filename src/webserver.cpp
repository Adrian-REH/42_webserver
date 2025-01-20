
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <poll.h>
#include <sys/wait.h>
#include <iostream>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <iostream>
#include <string>
#include <map>

#define MAX_CLIENTS 10

class Request {
	private:
		std::string url;
		std::string method;
		std::string body;
		std::map<std::string, std::string> headers;
	public:
};

std::string ft_readFile(int fd) {
    int n_byte = 0;
    std::string result = "HTTP/1.1 200 OK\r\n";
    char buffer[1024];
    do {
        n_byte = read(fd, buffer, sizeof(buffer));
        if (n_byte > 0)
            result.append(buffer, n_byte);
        return result;
    } while (n_byte > 0);
    return result;
}

pid_t execute_cgi(int *io, char *args[], char **env ) {

    pid_t pid = fork();

    if (pid < 0)
        return 0;
    else if (pid == 0) {
        dup2(io[0], 0); // Redirigir la entrada estándar al pipe
        close(io[0]);
        dup2(io[1], 1); // Redirigir la salida estándar al pipe
        close(io[1]);
        execve(args[0], args, env);
		exit(1);
    }
	return pid;
}

void ft_create_pid(struct epoll_event* clients, char** env, std::string body) {

    pid_t pid = fork();
    int status;
    int io[2];
    char* argv[] = {(char*)"/usr/bin/python3", (char*)"cgi-bin/app.py", (char *)body.c_str(), NULL};

    if (pid == 0) {
        if (pipe(io) < 0)
            exit(1);
        pid_t pidcgi = execute_cgi( io, argv, env);
		waitpid(pid, NULL, 0);
		std::string result = ft_readFile((io[0]));
        close(io[0]);
        close(io[1]);
        ssize_t val = send(clients->data.fd, result.c_str(), result.size(), 0); 
        close(clients->data.fd);
        clients->data.fd = -1;
        exit(1);
    }
	close(io[0]); // Cerrar lectura en el pipe
	close(io[1]); // Cerrar escritura

    waitpid(pid, &status, 0);
	close(clients->data.fd);
}

int main(int argc, char **argv, char **env) {
    int server_fd, epoll_fd, new_socket;
	std::string request;
	int bytes_received;
    struct sockaddr_in address;
    struct epoll_event ev, events[MAX_CLIENTS];
    socklen_t addrlen = sizeof(address);
    char buffer[1024];
    int opt = 1;
	int solicitude = 0;

    // Crear el socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar opciones del socket
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configurar dirección del socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Asociar socket con la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al hacer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) < 0) {
        perror("Error al hacer listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Crear el epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error al crear epoll");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Registrar el socket del servidor en epoll
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("Error al registrar el socket del servidor en epoll");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto 8080...\n");

    while (1) {
        // Esperar eventos en epoll
        int nfds = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
        if (nfds == -1) {
            perror("Error en epoll_wait");
            close(server_fd);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server_fd) {
                // Aceptar nueva conexión
                new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                if (new_socket == -1) {
                    perror("Error al aceptar la conexión");
                    continue;
                }
                // Registrar el nuevo socket en epoll
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) == -1) {
                    perror("Error al registrar el nuevo socket en epoll");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, new_socket, NULL);
                }
            } else {
                // Manejar actividad en el cliente
				/*  while ((bytes_received = recv(events[i].data.fd, buffer, sizeof(buffer), 0)) > 0) {
					request.append(buffer, bytes_received);
					// Si el cliente envía menos datos que el buffer, hemos terminado
					if (bytes_received <= (ssize_t)sizeof(buffer)) {
						break;
					}
				} */
				std::string request;
				int bytes = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
				request.append(buffer, bytes);
				//Build Request function

				size_t content_length = 0;
				size_t cl_pos = request.find("Content-Length: ");
				char *endp;
				if (cl_pos != std::string::npos) {
					content_length = strtoul(request.substr(cl_pos + 16).c_str(), &endp, 10);
				}
				std::cout << request.substr(0, request.find("\n")) << std::endl;
				std::string body = request.substr(request.size() - content_length);
				if (bytes_received < 0) {
                    printf("Cliente desconectado, socket fd: %d\n", events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                } else {
					size_t cl_pos = request.find("Content-Length: ");
					//std::cout <<"REQUEST: " << buffer<< std::endl;
                   // printf("Mensaje recibido en socket fd %d: %s\n", events[i].data.fd, buffer);
					if (strstr(request.c_str(), "GET /favicon.ico"))
					{
						const char* response = "HTTP/1.1 204 No Content\r\n\r\n";
						send(events[i].data.fd, response, strlen(response), 0); // Responder con 204
						close(events[i].data.fd);
					}
					else if (strstr(request.c_str(), "POST")){
						std::cout << "body: "<< body << std::endl;
						ft_create_pid(&(events[i]), env,body);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					}
					else if (strstr(request.c_str(), "GET")){
						std::cout << " body: "<< body << std::endl;
						ft_create_pid(&(events[i]), env, body);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					}
					else if (strstr(request.c_str(), "DELETE")){
						//Request request = parser_request(buffer);
						//std::cout << request << std::endl;
						//ft_create_pid(&(events[i]), env);
					}

                }
            }
        }
    }
close(server_fd);
close(epoll_fd);

    return 0;
}
