#include "GAPInstance.h"
#include <fstream>
#include <stdexcept>

GAPInstance::GAPInstance(const std::string& filename) {
    leer_archivo(filename);
}

void GAPInstance::leer_archivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }

    file >> m >> n;

    // Reservar matrices m x n
    costos.assign(m, std::vector<double>(n));
    demandas.assign(m, std::vector<double>(n));
    capacidades.resize(m);

    // Leer matriz de costos costos[i][j]: m filas, n columnas
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++) {
            file >> costos[i][j];
            if (costos[i][j] > costo_max)
                costo_max = costos[i][j];
        }

    // Leer matriz de consumos demandas[i][j]: m filas, n columnas
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            file >> demandas[i][j];

    // Leer capacidades capacidades[i]: m valores
    for (int i = 0; i < m; i++)
        file >> capacidades[i];

    if (file.fail() && !file.eof()) {
        throw std::runtime_error("Error al leer el archivo: datos incompletos o malformados.");
    }
}