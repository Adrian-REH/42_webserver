

#include "Cookie.hpp"


Cookie::Cookie() : _session_id(""), _expiration(0), _session("")
{}
// Constructor
Cookie::Cookie(const std::string& session_id, const std::string &session)
	: _session_id(session_id), _expiration(std::time(0) + (1 * 24 * 60 * 60)), _session(session)
{}

// Getter para session_id
const std::string& Cookie::get_session_id() const {
	return _session_id;
}

// Getter para expiration
time_t Cookie::getExpiration() const {
	return _expiration;
}

Cookie& Cookie::operator=(const Cookie & cook) {
	if (this == &cook)
		return (*this);
	this->_session = cook._session;
	this->_expiration = cook._expiration;
	this->_session_id = cook._session_id;
	return *this;
}
// Método para verificar si la cookie ha expirado
bool Cookie::isExpired() const {
	return std::time(NULL) > _expiration;
}

Cookie& Cookie::set_session(const std::string & session) {
	_session = session;
	return *this;
}

std::string Cookie::get_session() const {
	return _session;
}

// Método para renovar la cookie
void Cookie::renew(time_t new_expiration) {
	_expiration = new_expiration;
}
bool Cookie::empty() {
	return (_expiration==0 && _session_id.empty());
}



