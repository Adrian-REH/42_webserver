#include "Utils.hpp"


/**
 * @brief Verifica si una cadena comienza con un prefijo específico.
 * 
 * @param str Cadena principal.
 * @param prefix Prefijo a comprobar.
 * @return true Si la cadena comienza con el prefijo.
 * @return false En caso contrario.
 */
bool starts_with(const std::string& str, const std::string& prefix) {
    if (str.size() < prefix.size()) {
        return false;  
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}
