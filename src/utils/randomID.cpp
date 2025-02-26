#include "Utils.hpp"
// Función para generar un número aleatorio en el rango [min, max]
int randomInRange(int min, int max) {
    return std::rand() % (max - min + 1) + min;
}

// Función para generar un session_id alfanumérico
std::string generateSessionID(int length) {
    const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string sessionID;

    // Inicializar semilla para aleatoriedad
    std::srand(static_cast<unsigned int>(std::time(0)));

    for (int i = 0; i < length; ++i) {
        int randomIndex = randomInRange(0, characters.size() - 1);
        sessionID += characters[randomIndex];
    }

    return sessionID;
}

