#ifndef LIMITEXCEPT_HPP
#define LIMITEXCEPT_HPP
#include <string>
#include <set>

class  LimitExcept {
	private:
		std::set<std::string> _allowedMethods; // Métodos permitidos
		std::string _denyAction; // Acción a tomar si no está permitido
		std::string _logMessage; // Mensaje de log (opcional)
		std::string _authMessage; // Mensaje de autenticación (opcional)

	public:
		LimitExcept() {}
		// Constructor
		LimitExcept(const std::vector<std::string>& methods, const std::string& denyAction = "deny all")
			: _allowedMethods(methods.begin(), methods.end()), _denyAction(denyAction) {}

		// Agregar un método permitido
		LimitExcept addAllowedMethod(const std::string& method) {
			_allowedMethods.insert(method);
			return *this;
		}

		// Configurar acciones de denegación
		LimitExcept setDenyAction(const std::string& action) {
			_denyAction = action;
			return *this;
		}

		// Configurar mensaje de log
		LimitExcept setLogMessage(const std::string& message) {
			_logMessage = message;
			return *this;
		}

		// Configurar mensaje de autenticación
		LimitExcept setAuthMessage(const std::string& message) {
			_authMessage = message;
			return *this;
		}

		// Verificar si un método está permitido
		bool isMethodAllowed(const std::string& method) const {
			return _allowedMethods.find(method) != _allowedMethods.end();
		}

};

#endif