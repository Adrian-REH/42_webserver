#include "Utils.hpp"

/**
 * @brief Verifica si una cadena termina con un sufijo específico.
 * 
 * @param str Cadena principal.
 * @param suffix Sufijo a comprobar.
 * @return true Si la cadena termina con el sufijo.
 * @return false En caso contrario.
 */
bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) {
        return false;  // No puede terminar con el sufijo si es más pequeño
    }
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}
