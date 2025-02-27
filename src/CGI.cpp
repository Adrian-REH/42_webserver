
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

int CGI::parse_request_details(std::map<std::string, std::string> headers) {
	//Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryExBsdaWEWoLMf00z
	if (headers["Content-Type"].empty()) return 0;
	std::deque<std::string>	content_type = split(headers["Content-Type"], ';');
	std::string boundary;
	std::cout <<  "'" << strtrim(content_type[0]) <<  "'"<< std::endl;
	/* if (_method == "POST" && strtrim(content_type[0]) == "multipart/form-data")
	{
		boundary = split(content_type[1], '=')[1];
		_body = extractStrBetween(_body, boundary + "\r\n", boundary + "--\r\n");
	} */
	
	return 0;
}
/**
 * @brief Ejecuta el script CGI y devuelve su salida.
 * 
 * 
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
		try {
			std::string interpreter = determine_interpreter();
			char* argv[] = {
				(char*)interpreter.c_str(),
				(char*)_script_path.c_str() + 1, // Quito el primer caracter '/'
				NULL
			};
			dup2(io[1], STDOUT_FILENO);
			dup2(io[1], STDERR_FILENO);
			close(io[1]);
			dup2(io[0], STDIN_FILENO);
			close(io[0]);

			execve(interpreter.c_str(), argv, _env);
			exit(4);
		} catch (const std::exception&  ) {
			exit(2);
		}
	} else {
		//size_t bytes_written = 0;
		std::cout<< "BOOOODYY: " << _body << std::endl;
		write(io[1], _body.c_str(), _body.size() );
		close(io[1]);
		// Padre: Leer la salida del hijo
		waitpid(pid, &status, 0);
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
		std::string result = readFd(io[0]);
		if (ret) {
			close(io[0]);
			std::string error(strerror(ret));
			throw std::runtime_error("Error en la ejecucion del CGI, Error : " + to_string(ret) + " " +error + ", script_path: " + _script_path + ", body:" + _body + ", result: " + result);
		}
		close(io[0]);
		std::cout << "RESULT: "<< result << std::endl;
		return (result);
	}
}
