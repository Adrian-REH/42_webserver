
#include "SessionCookieManager.hpp"

SessionCookieManager::SessionCookieManager(){

}

SessionCookieManager& SessionCookieManager::getInstance() {
	static SessionCookieManager instance;
	return instance;
}

bool SessionCookieManager::isExpired(const Cookie& cookie) {
	std::time_t currentTime = std::time(0);
	return difftime(currentTime, cookie.expiration) > 0;
}

Cookie SessionCookieManager::setCookieBySessionId(const std::string& session_id, int expirationInSeconds) {
	std::time_t currentTime = std::time(0);
	removeExpiredCookies();
	if (getCookieBySessionId(session_id).isEmpty())
		return Cookie();
	std::time_t expirationTime = currentTime + expirationInSeconds;
	Cookie cookie;
	cookie.name = "session_id";
	cookie.value = session_id;
	cookie.expiration = expirationTime;
	_cookies[session_id] = cookie;
	return cookie;
}

Cookie SessionCookieManager::getCookieBySessionId(const std::string& session_id) {
	if (_cookies.find(session_id) != _cookies.end()) {
		Cookie& cookie = _cookies[session_id];
		if (isExpired(cookie)) {
			_cookies.erase(session_id);
			return Cookie();
		}
		return cookie;
	}
	return Cookie();
}

void SessionCookieManager::deleteCookieBySessionId(const std::string& session_id) {
	_cookies.erase(session_id);
}

bool SessionCookieManager::isCookieExpired(const Cookie& cookie) {
		return isExpired(cookie);
}

bool SessionCookieManager::isCookieExpiredBySessionId(const std::string& session_id) {
	if (_cookies.find(session_id) != _cookies.end()) {
		return isExpired(_cookies[session_id]);
	}
	return true;  // Si no existe, consideramos que está "expirada"
}

void SessionCookieManager::removeExpiredCookies() {
	std::map<std::string, Cookie>::iterator it;
	for (it = _cookies.begin(); it != _cookies.end(); it++) {
		if (isExpired(it->second)) {
			_cookies.erase(it);
			it = _cookies.begin();
		}
	}
}
