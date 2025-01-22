#ifndef CGI_HPP
#define CGI_HPP
#include <string>
#include <sys/wait.h>
#include "utils/readFd.hpp"

/**
 * @brief Verifica si una cadena termina con un sufijo específico.
 * 
 * @param str Cadena principal.
 * @param suffix Sufijo a comprobar.
 * @return true Si la cadena termina con el sufijo.
 * @return false En caso contrario.
 */
bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) {
        return false;  // No puede terminar con el sufijo si es más pequeño
    }
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief Clase para gestionar la ejecución de scripts CGI.
 * 
 * La clase `CGI` permite configurar y ejecutar scripts CGI en el servidor, 
 * determinando el intérprete adecuado según el tipo de archivo.
 */
class CGI {
private:
    std::string _script_path;
    std::string _method;
    std::string _body;
    char** _env;

public:
    /**
     * @brief Constructor de la clase CGI.
     * 
     * @param script_path Ruta al script a ejecutar.
     * @param method Método HTTP asociado al script.
     * @param body Cuerpo de la solicitud HTTP.
     * @param env Variables de entorno para la ejecución.
     */
    CGI(const std::string& script_path, const std::string& method, const std::string& body, char** env)
        : _script_path(script_path), _method(method), _body(body), _env(env) {}

    /**
     * @brief Determina el intérprete adecuado para el script según su extensión.
     * 
     * @return Ruta al ejecutable del intérprete correspondiente.
     * @throws std::runtime_error Si el tipo de script no está soportado.
     */
    std::string determine_interpreter() const {
        if (ends_with(_script_path, ".py")) {
            return "/usr/bin/python3";
        } else if (ends_with(_script_path, ".php")) {
            return "/usr/bin/php";
        } else if (ends_with(_script_path, ".js")) {
            return "/usr/bin/node";
        } else {
            throw std::runtime_error("Unsupported script type: " + _script_path);
        }
    }
    /**
     * @brief Ejecuta el script CGI y devuelve su salida.
     * 
     * Crea un proceso hijo que ejecuta el script CGI utilizando el intérprete correspondiente.
     * Captura la salida del script y la devuelve como una respuesta HTTP.
     * 
     * @return Respuesta HTTP generada por el script CGI.
     * @throws std::runtime_error Si falla la creación del pipe o el fork.
     */
    std::string execute() {
        int status;
        int io[2];
        if (pipe(io) < 0) {
            throw std::runtime_error("No se pudo crear un pipe.");
        }

        pid_t pid = fork();
        if (pid < 0) {
            throw std::runtime_error("Error al hacer fork.");
        }

        if (pid == 0) {
            // Hijo: Configurar el entorno para la ejecución del CGI
            close(io[0]);
            dup2(io[1], STDOUT_FILENO);
            dup2(io[1], STDERR_FILENO);
            close(io[1]);
            try {
                // Determinar el intérprete según el tipo de script
                std::string interpreter = determine_interpreter();

                char* argv[] = {
                    (char*)interpreter.c_str(),
                    (char*)_script_path.c_str() + 1, // Quito le primer caracter '/'
                    (char*)_body.c_str(),
                    NULL
                };

                execve(interpreter.c_str(), argv, _env);
                perror("execve");
                exit(EXIT_FAILURE);
            } catch (const std::exception&  ) {
                exit(EXIT_FAILURE);
            }
        } else {
            // Padre: Leer la salida del hijo
            close(io[1]);
            waitpid(pid, &status, 0);
            
            status = WEXITSTATUS(status);
            //TODO: Intentar devolver multiples codigos de error
            std::string result = "HTTP/1.1 200 OK\r\n";
            if (status)
                return (close(io[0]), "HTTP/1.1 500 Server Internal Error\r\n");
            result.append(readFd(io[0]));
            return (close(io[0]), result);
        }
    }
};

#endif