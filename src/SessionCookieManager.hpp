#ifndef SESSIONCOOKIEMANAGER_HPP
#define SESSIONCOOKIEMANAGER_HPP

#include "Cookie.hpp"
#include <string>
#include <map>

class SessionCookieManager {
	private:
		std::map<std::string, Cookie> _cookies; // key: session_id value:Cookie
		/**
		 * @brief Método auxiliar para verificar si la cookie ha expirado
		 */
		bool isExpired(const Cookie& cookie);
		SessionCookieManager();
		SessionCookieManager(SessionCookieManager&);
		SessionCookieManager& operator=(SessionCookieManager&){return *this;}
	public:
		static SessionCookieManager& getInstance();
		/**
		 * @brief Establece una cookie de sesión con un valor y tiempo de expiración
		 */
		Cookie setCookieBySessionId(const std::string& session_id, int expirationInSeconds);
		/**
		 * @brief Obtiene el valor de una cookie de sesión si no ha expirado
		 */
		Cookie getCookieBySessionId(const std::string& session_id);
		/**
		 * @brief Elimina una cookie de sesión en base a su session_id
		 */
		void deleteCookieBySessionId(const std::string& session_id);
		/**
		 * @brief  si la cookie ha expirado
		 */
		bool isCookieExpiredBySessionId(const std::string& session_id);
			/**
		 * @brief Elimino todas las cookies que estan expiradas
		 */
		void removeExpiredCookies();
		bool isCookieExpired(const Cookie& cookie);
};


#endif