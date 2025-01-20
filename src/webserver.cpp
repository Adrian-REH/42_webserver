
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

/**
 * @brief Represents an HTTP request with its components such as URL, method, body, and headers.
 *
 * The `Request` class encapsulates the details of an HTTP request, including:
 * - The URL being requested.
 * - The HTTP method (e.g., GET, POST, PUT, DELETE).
 * - The body of the request (used in methods like POST).
 * - Headers in the form of key-value pairs.
 */
class Request {
	private:
		std::string url;
		std::string method;
		std::string body;
		std::map<std::string, std::string> headers;
	public:
};

/**
 * @brief Reads the contents of a file descriptor and returns it as a string.
 *
 * This function reads data from the provided file descriptor (fd) in chunks of
 * 1024 bytes and appends the content to a string. The resulting string starts
 * with the HTTP header "HTTP/1.1 200 OK\r\n" harcoded. 
 *
 * @param fd The file descriptor from which data will be read.
 * @return A std::string containing the HTTP response header and the content
 *         read from the file descriptor. If no data is read, the string will 
 *         contain only the HTTP header.
 *
 * @note The function currently has a logical issue because it includes a 
 *       `return result;` inside the `do-while` loop, which prevents the loop 
 *       from iterating beyond the first read. This should be corrected if 
 *       multiple reads are expected.
 */
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

/**
 * @brief Executes a CGI script by forking a new process and setting up I/O redirection.
 *
 * This function forks a new process to execute a CGI (Common Gateway Interface) script.
 * It redirects the input and output of the child process to the provided pipes 
 * and executes the script using `execve`.
 *
 * @param io An array of two file descriptors for input and output pipes:
 *           - `io[0]`: File descriptor for reading (stdin for the CGI script).
 *           - `io[1]`: File descriptor for writing (stdout for the CGI script).
 * @param args An array of arguments for the CGI script. The first element should
 *             be the path to the script, followed by its arguments, and terminated
 *             with a `NULL` pointer.
 * @param env An array of environment variables to be passed to the CGI script,
 *            terminated with a `NULL` pointer.
 *
 * @return The process ID (`pid_t`) of the forked child process, or 0 if the fork
 *         fails.
 *
 * @note The function closes the pipes in the child process after duplicating them
 *       to standard input and output. In the parent process, the pipes remain open,
 *       and the caller is responsible for managing and closing them.
 *
 * @note If `execve` fails, the child process will exit with a status of 1.
 */
pid_t execute_cgi(int *io, char *args[], char **env ) {

    pid_t pid = fork();

    if (pid < 0)
        return 0;
    else if (pid == 0) {
        dup2(io[0], 0);
        close(io[0]);
        dup2(io[1], 1);
        close(io[1]);
        execve(args[0], args, env);
		exit(1);
    }
	return pid;
}

/**
 * @brief Creates a child process to execute a CGI script and sends the result back to the client.
 *
 * This function forks a new process to handle the execution of a CGI script using `execute_cgi`. 
 * It sets up communication with the CGI script using pipes and sends the script's output 
 * back to the client through the provided epoll event.
 *
 * @param clients A pointer to an `epoll_event` structure representing the client connection.
 *                The file descriptor (`clients->data.fd`) is used for sending data back to the client.
 * @param env A `char**` array representing the environment variables to be passed to the CGI script.
 * @param body A `std::string` containing the request body or additional data to be passed as an argument
 *             to the CGI script.
 *
 * @note The function:
 *       - Creates a pipe for communication between the parent and child processes.
 *       - Uses `execute_cgi` to execute the CGI script in the child process.
 *       - Reads the CGI script's output from the pipe.
 *       - Sends the output to the client and closes the client's file descriptor.
 *       - Properly waits for child processes to avoid zombie processes.
 *
 * @note The function closes the pipe file descriptors and the client socket after use to free resources.
 */
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
