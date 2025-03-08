#include "Utils.hpp"
/**
 * @brief Convierte un valor entero a una cadena de texto.
 * 
 * @param value El valor entero a convertir.
 * @return std::string Representación en cadena del entero.
 */
std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
