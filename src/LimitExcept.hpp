#ifndef LIMITEXCEPT_HPP
#define LIMITEXCEPT_HPP
#include <string>
#include <set>
#include <vector>

class  LimitExcept {
	private:
		std::set<std::string> _allowedMethods; // Métodos permitidos
		std::string _denyAction; // Acción a tomar si no está permitido
		std::string _logMessage; // Mensaje de log (opcional)
		std::string _authMessage; // Mensaje de autenticación (opcional)

	public:
		LimitExcept();
		// Constructor
		LimitExcept(const std::vector<std::string>& methods, const std::string& denyAction = "deny all");

		// Agregar un método permitido
		LimitExcept &addAllowedMethod(const std::string& method);

		// Configurar acciones de denegación
		LimitExcept &setDenyAction(const std::string& action);

		// Configurar mensaje de log
		LimitExcept &setLogMessage(const std::string& message);

		// Configurar mensaje de autenticación
		LimitExcept &setAuthMessage(const std::string& message);

		// Verificar si un método está permitido
		bool isMethodAllowed(const std::string& method) const;

		std::string allowed_methods_to_str();

		class LimitExceptionNotAllowedMethodException: public std::exception {
			public:
				const char* what() const throw() {
					return "Limit Exception not allowed method";
				}
		};
};

#endif