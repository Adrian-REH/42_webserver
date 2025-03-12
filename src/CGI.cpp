
#include "CGI.hpp"
#include "Request.hpp"
#include "Server.hpp"


CGI::CGI(const std::string& working_dir, const std::string& script_path, Request request, char** env, size_t exec_timeout)
	: _working_dir(working_dir) ,_script_path(script_path), _request(request), _env(env),_exec_timeout(exec_timeout), _interpreter(determine_interpreter()),_status_code(200) {
		determine_interpreter();
	}
CGI::~CGI() {
	
	if (_env) {
		for (int i = 0; _env[i] != 0; i++)
			delete [] _env[i];
		delete [] _env;
	}
}

int CGI::get_status_code() {
	return _status_code;
}
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
		throw HttpException::UnsupportedMediaTypeException("Unsupported script type: " + _script_path);
	}
}

int CGI::resolve_cgi_env(Request req, std::string http_cookie) {
	std::vector<std::string> env_strings;
	env_strings.push_back(std::string(http_cookie));
	env_strings.push_back("REDIRECT_STATUS=200" );
	env_strings.push_back("REQUEST_METHOD=" + req.get_method());
	env_strings.push_back("QUERY_STRING=" + req.get_query_string());
	env_strings.push_back("CONTENT_LENGTH=" + req.get_header_by_key("Content-Length")); // Como lee Webserver por chunked el body entonces no hare que CGI se encarge de leerlo.
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
		_env[i] = new char[env_strings[i].size() + 1];
		std::strcpy(_env[i], env_strings[i].c_str());
	}
	_env[env_strings.size()] = 0;  // El último elemento debe ser NULL
	
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
	int cgi_io[2];
	int cgi_response[2];
	Logger::log(Logger::DEBUG, "CGI.cpp", "Executing CGI...");

	if (pipe(cgi_io) < 0)
		throw HttpException::InternalServerErrorException();
	if (pipe(cgi_response) < 0) {
		closeFDs(cgi_io);
		throw HttpException::InternalServerErrorException();
	}
		
	pid_t pid = fork();
	if (pid < 0) {
		closeFDs(cgi_response);
		closeFDs(cgi_io);
		throw HttpException::InternalServerErrorException();
	}
	
	if (pid == 0) {
		if (chdir(_working_dir.c_str()) == -1) {
			closeFDs(cgi_response);
			closeFDs(cgi_io);
			exit(errno);
		}
		char* argv[] = {
			(char*)_interpreter.c_str(),
			(char*)_script_path.c_str(),
			NULL
		};
		std::cout << argv[0]<< std::endl;
		std::cout << argv[1]<< std::endl;
		std::cout << argv[2]<< std::endl;
		if (dup2(cgi_response[1], STDOUT_FILENO) < 0)
			(closeFDs(cgi_response), closeFDs(cgi_io), exit(errno));
		if (dup2(cgi_io[0], STDIN_FILENO) < 0)
			(closeFDs(cgi_response), closeFDs(cgi_io), exit(errno));
		
		closeFDs(cgi_response);
		closeFDs(cgi_io);
		execve(_interpreter.c_str(), argv, _env);
		exit(errno);
	} else {
		close(cgi_io[0]);
		Logger::log(Logger::DEBUG,"CGI.cpp","Body Writing.. size:" + to_string(_request.get_body().size()));
		size_t chunk_size = 8192;
		size_t written = 0;

		while (written < _request.get_body().size()) {
			size_t to_write = std::min(chunk_size, _request.get_body().size() - written);
			ssize_t bytes_written = write(cgi_io[1], _request.get_body().c_str() + written, to_write);
			if (bytes_written <= 0) break;
			written += bytes_written;
		}

		close(cgi_io[1]);
		close(cgi_response[1]);
		fd_set set;
		struct timeval timeout;
		timeout.tv_sec = _exec_timeout;
		timeout.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(cgi_response[0], &set);
		/**
		 * Escucho el estado del fd io[0] +1, en caso de que cambie su estado(alguienn escriba sobre el) antes de timeout entonces result = numero de fds escritos
		 * Si en caso de que no se escriba sobre el hasta o luego de llegar a timeout, select devolvera 0fds escritos y ejecutare un error
		 */
		Logger::log(Logger::DEBUG,"CGI.cpp","Service waiting to pid: " + to_string(pid));
		int nfds = select(cgi_response[0] + 1, &set, NULL , NULL, &timeout);
		Logger::log(Logger::DEBUG,"CGI.cpp","pid: " + to_string(pid) + " response nfds: " + to_string(nfds));
		if (nfds == 0) {
			kill(pid, SIGKILL);
			close(cgi_response[0]);
			throw HttpException::RequestTimeoutException("Timeout CGI execution");
		} else if (nfds < 0) {
			close(cgi_response[0]);
			throw HttpException::InternalServerErrorException();
		}
			
		//Obtengo el estado del pid
		waitpid(pid, &status, 0);
		int ret = 0;
		if (WIFEXITED(status)){
			ret = WEXITSTATUS(status);
			Logger::log(Logger::WARN,"CGI.cpp", "WEXITSTATUS "+ to_string(ret));
		}
		if (WIFSIGNALED(status)){
			ret = WTERMSIG(status);
			Logger::log(Logger::WARN,"CGI.cpp", "WTERMSIG "+ to_string(ret));
		}

		std::string result;
		size_t  expired = _exec_timeout + std::time(0);
		char buffer[1024];
		ssize_t bytes_read;
	
		while ((bytes_read = read(cgi_response[0], buffer, sizeof(buffer))) > 0) {
			result.append(buffer, bytes_read);
			std::time_t currentTime = std::time(0);
			if (difftime(currentTime, expired) > 0) {
				close(cgi_response[0]);
				throw HttpException::RequestTimeoutException();
			}	
		}
		close(cgi_response[0]);

		if (ret) {
			std::string error(strerror(ret));
			 Logger::log(Logger::ERROR,"CGI.cpp", "Error en la ejecucion del CGI, Error : " + to_string(ret) + " " +error + ", script_path: " + _script_path + ", body:" + _request.get_body() + ", result: " + result);
			switch (ret)
			{
				case 1	: _status_code = 502; break;
				case 13	: _status_code = 403; break;
				case 2	: _status_code = 404; break;
				case 22	: _status_code = 400; break;
				case 95	: _status_code = 405; break;
				case 112: _status_code = 302; break;
				default	: _status_code = 500; break;
			}
		}
		return (result);
	}
}

