
#include "LimitExcept.hpp"

LimitExcept::LimitExcept() : _allowedMethods(), _denyAction(""), _logMessage(""), _authMessage("") {}

LimitExcept::LimitExcept(const std::vector<std::string>& methods, const std::string& denyAction)
		: _allowedMethods(methods.begin(), methods.end()), _denyAction(denyAction) {}

// Agregar un método permitido
LimitExcept &LimitExcept::addAllowedMethod(const std::string& method) {
	_allowedMethods.insert(method);
	return *this;
}

// Configurar acciones de denegación
LimitExcept &LimitExcept::setDenyAction(const std::string& action) {
	_denyAction = action;
	return *this;
}

// Configurar mensaje de log
LimitExcept &LimitExcept::setLogMessage(const std::string& message) {
	_logMessage = message;
	return *this;
}

// Configurar mensaje de autenticación
LimitExcept &LimitExcept::setAuthMessage(const std::string& message) {
	_authMessage = message;
	return *this;
}

// Verificar si un método está permitido
bool LimitExcept::isMethodAllowed(const std::string& method) const {
	return _allowedMethods.find(method) != _allowedMethods.end();
}

std::string LimitExcept::allowed_methods_to_str(){
	std::string methods;

	std::set<std::string>::iterator it;
	for (it = _allowedMethods.begin(); it != _allowedMethods.end(); it++)
		methods += (*it) + ", ";
	methods.erase(methods.rfind(", "), 2);
	return methods;	
}

