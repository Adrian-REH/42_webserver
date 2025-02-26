#ifndef COOKIE_HPP
#define COOKIE_HPP

#include "utils/Utils.hpp"

class Cookie {
private:
	std::string _session_id;
	std::time_t _expiration;
	std::string _session;

public:
	Cookie();
	// Constructor
	Cookie(const std::string& session_id, const std::string &session);
	// Getter para session_id
	const std::string& get_session_id() const;
	// Getter para expiration
	time_t getExpiration() const;
	Cookie& operator=(const Cookie & cook);
	// Método para verificar si la cookie ha expirado
	bool isExpired() const;
	Cookie& set_session(const std::string & session);
	std::string get_session() const;
	// Método para renovar la cookie
	void renew(time_t new_expiration);
	bool empty();
};



#endif