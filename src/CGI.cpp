
#include "CGI.hpp"


CGI::CGI(const std::string& script_path, const std::string& method, const std::string& body, char** env)
	: _script_path(script_path), _method(method), _body(body), _env(env) {}

/**
 * @brief Determina el intérprete adecuado para el script según su extensión.
 * 
 * @return Ruta al ejecutable del intérprete correspondiente.
 * @throws std::runtime_error Si el tipo de script no está soportado.
 */
std::string CGI::determine_interpreter() const {
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
std::string CGI::execute() {
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
			exit(4);
		} catch (const std::exception&  ) {
			exit(2);
		}
	} else {
		// Padre: Leer la salida del hijo
		close(io[1]);
		waitpid(pid, &status, WUNTRACED);
		//status = WEXITSTATUS(status);
		int ret;
		if (WIFEXITED(status)){
			ret = WEXITSTATUS(status);
			//Logger::log(Logger::WARN,"CGI.cpp", "WEXITSTATUS "+ to_string(ret));
		}
			
		if (WIFSIGNALED(status)){
			ret = WTERMSIG(status);
			//Logger::log(Logger::WARN,"CGI.cpp", "WTERMSIG "+ to_string(ret));
		}
			
		// TODO: Intentar devolver multiples codigos de error
		if (ret) {
			close(io[0]);
			std::string error(strerror(ret));
			throw std::runtime_error("Error en la ejecucion del CGI, Error : " + to_string(ret) + " " +error + ", script_path: " + _script_path + ", body:" + _body);
		}
		std::string result = readFd(io[0]);
		close(io[0]);
		return (result);
	}
}
