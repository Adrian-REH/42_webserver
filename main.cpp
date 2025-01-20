
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <poll.h>
#include <sys/wait.h>
#include <iostream>

#define MAX_CLIENTS 10

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
	const char* post_data = "nombre=adr&edad=10"; // Datos que quieres enviar
	size_t post_data_len = strlen(post_data);


    if (pid < 0)
        return 0;
    else if (pid == 0) {
        std::cout << io[0] << std::endl;
        dup2((io[0]), 0);
        close(io[0]);
        dup2((io[1]), 1);
        close(io[1]);
        execve(args[0], args, env);
        exit(1);
    }
	write(io[1], post_data, post_data_len); // Enviar los datos al pipe

    return pid;
}

void ft_create_pid(struct pollfd* clients, char** env) {

    pid_t pid = fork();
    int status;
    int io[2];
    char* argv[] = {(char*)"/usr/bin/python3", (char*)"app.py", NULL};

    if (pid == 0) {
        const char response[] = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "\r\n"
                                "<h1>Hola Grace!!!</h1>\0";
        if (pipe(io) < 0)
            exit(1);
        pid_t pidcgi = execute_cgi( io, argv, env);
        //Time wait of cgi
        waitpid(pidcgi, &status, 0);

        std::string result = ft_readFile((io[0]));

        ssize_t val = send(clients->fd, result.c_str(), result.size(), 0); 

        close(clients->fd);
        clients->fd = -1;
    }
    waitpid(pid, &status, -1);
    std::cout << WEXITSTATUS(status) << std::endl;
}

int main(int argc, char **argv, char** env) {
    (void)argc;
    (void)argv;
    (void)env;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

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

	printf("Servidor escuchando en el puerto 8080...\n");
	char buffer[1024];

	//Handling Backlog
	struct pollfd clients[MAX_CLIENTS];
	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].fd = -1;
		clients[i].events = 0;
	}
    clients[0].fd = server_fd;
    clients[0].events = POLLIN;
	(void)clients;

	while (1) {
        // Esperar actividad en los sockets
        int activity = poll(clients, MAX_CLIENTS, -1); // Espera indefinida
        if (activity < 0) {
            perror("Error en poll");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

       // Verificar actividad en el socket del servidor
        if (clients[0].revents & POLLIN) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket < 0) {
                perror("Error al aceptar la conexión");
                continue;
            }

            printf("Nueva conexión aceptada, socket fd: %d, IP: %s, puerto: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Agregar el nuevo socket al array de `pollfd`
            int added = 0;
            for (int i = 1; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) { // Buscar espacio libre
                    clients[i].fd = new_socket;
                    clients[i].events = POLLIN; // Monitorear para lectura
                    //Puedo dejarlo en cola
                    added = 1;
                    break;
                }
            }

            if (!added) {
                printf("Demasiados clientes. Cerrando conexión.\n");
                close(new_socket);
            }
        }

        //PORQUE TENGO QUE RECORRER TODOS LOS CLIENTES SIEMPRE QUE ENCUENTRE UNA CONEXION
        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == -1) continue; // Saltar sockets inactivos

            if (clients[i].revents & POLLIN) {
                //Luego de recibie el FIN notifica al fd y lo recibo.
                ssize_t bytes_received = recv(clients[i].fd, buffer, sizeof(buffer), 0);
                if (bytes_received <= 0) {
                    std::cout << "Cliente desconectado, socket fd: "<< clients[i].fd <<" | " << buffer << std::endl;
                    close(clients[i].fd);
                    clients[i].fd = -1; // Liberar espacio
                } else {
                    std::cout << "Ejecutando socket fd: " << clients[i].fd << " | " << buffer << std::endl;
                    buffer[bytes_received] = '\0'; // Asegurar que sea un string
                    //std::cout << buffer << std::endl;
                    
                    //Handling methods of HTTP (POST, GET, PUT, DELETE...)
                    ft_create_pid(&(clients[i]), env);
                }
            }
            //Waiting clients
        }
    
    }
   
    return 0;
}