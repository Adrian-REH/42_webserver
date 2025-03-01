
#include "CGI.hpp"
#include "Request.hpp"


CGI::CGI(const std::string& working_dir, const std::string& script_path, Request request, char** env, size_t exec_timeout)
	: _working_dir(working_dir) ,_script_path(script_path), _request(request), _env(env),_exec_timeout(exec_timeout) {}

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

int CGI::resolve_cgi_env(Request req, std::string http_cookie) {
	std::vector<std::string> env_strings;
	env_strings.push_back(http_cookie);
	env_strings.push_back("REQUEST_METHOD=" + req.get_method());
	env_strings.push_back("QUERY_STRING=" + req.get_query_string());
	env_strings.push_back("CONTENT_LENGTH=" + req.get_header_by_key("Content-Length")); // Como lee Webserver por chunked el body entonces no hare que CGI se encarge de leerlo.
	env_strings.push_back("QUERY_STRING=" + req.get_query_string());
	env_strings.push_back("SCRIPT_NAME=" + _script_path);
	env_strings.push_back("CONTENT_TYPE=" + req.get_header_by_key("Content-Type"));
	env_strings.push_back("SERVER_NAME=" + req.get_header_by_key("Host"));
	env_strings.push_back("REMOTE_ADDR=" + req.get_header_by_key("X-Forwarded-For"));
	env_strings.push_back("SERVER_PORT=" + req.get_header_by_key("X-Forwarded-Port"));
	env_strings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_strings.push_back("HTTP_USER_AGENT=" + req.get_header_by_key("User-Agent"));
	env_strings.push_back("HTTP_REFERER=" + req.get_header_by_key("Referer"));
	env_strings.push_back("HTTP_ACCEPT_ENCODING=" + req.get_header_by_key("Accept-Encoding"));

	_env = new char*[env_strings.size() + 1];
	for (size_t i = 0; i < env_strings.size(); ++i) {
		_env[i] = (char*)env_strings[i].c_str();
	}
	_env[env_strings.size()] = NULL;  // El último elemento debe ser NULL
	
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
 * @throws HttpException::InternalServerErrorException Si falla la creación del pipe o el fork.
 */
std::string CGI::execute() {
	int status;
	int io[2];

	if (pipe(io) < 0)
		throw HttpException::InternalServerErrorException();

	pid_t pid = fork();
	if (pid < 0)
		throw HttpException::InternalServerErrorException();

	if (pid == 0) {
		try {
			if (chdir(_working_dir.c_str()) == -1) {
				exit(errno);// failure exit from CHILD PROCESS
			}

			std::string interpreter = determine_interpreter();
			char* argv[] = {
				(char*)interpreter.c_str(),
				(char*)_script_path.c_str() , // Quito el primer caracter '/'
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
		
		write(io[1], _request.get_body().c_str(), _request.get_body().size());
		close(io[1]);
		fd_set set;
		struct timeval timeout;
		timeout.tv_sec = _exec_timeout;
		timeout.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(io[0], &set);

		/**
		 * Escucho el estado del fd io[0] +1, en caso de que cambie su estado(alguienn escriba sobre el) antes de timeout entonces result = numero de fds escritos
		 * Si en caso de que no se escriba sobre el hasta o luego de llegar a timeout, select devolvera 0fds escritos y ejecutare un error
		 */
		int nfds = select(io[0] + 1, &set, NULL , NULL, &timeout);
		if (nfds == 0) {
			kill(pid, SIGKILL);
			throw HttpException::RequestTimeoutException("Timeout CGI execution");
		} else if (nfds < 0)
			throw HttpException::InternalServerErrorException();
		//Obtengo el estado del pid
		waitpid(pid, &status, 0);
		int ret;
		if (WIFEXITED(status)){
			ret = WEXITSTATUS(status);
			//Logger::log(Logger::WARN,"CGI.cpp", "WEXITSTATUS "+ to_string(ret));
		}
		if (WIFSIGNALED(status)){
			ret = WTERMSIG(status);
			//Logger::log(Logger::WARN,"CGI.cpp", "WTERMSIG "+ to_string(ret));
		}
		std::string result = readFd(io[0]);
		close(io[0]);
		if (ret) {
			std::string error(strerror(ret));
			 Logger::log(Logger::ERROR,"CGI.cpp", "Error en la ejecucion del CGI, Error : " + to_string(ret) + " " +error + ", script_path: " + _script_path + ", body:" + _request.get_body() + ", result: " + result);
			throw HttpException::InternalServerErrorException();
		}
		return (result);
	}
}
