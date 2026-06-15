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
    costos_por_vendedor.assign(n, std::vector<double>(m));
    
    demandas.assign(m, std::vector<double>(n));
    demandas_por_vendedor.assign(n, std::vector<double>(m));
    
    capacidades.resize(m);
  
    costo_max = 0;

    // Leer matriz de costos costos[i][j]: m filas, n columnas
    for (int d = 0; d < m; d++) {
        for (int v = 0; v < n; v++) {
            file >> costos[d][v];
            costos_por_vendedor[v][d] = costos[d][v];
            if (costos[d][v] > costo_max)
                costo_max = costos[d][v];
        }
    }
    // Leer matriz de consumos demandas[i][j]: m filas, n columnas
    for (int d = 0; d < m; d++) {
        for (int v = 0; v < n; v++){
            file >> demandas[d][v];
            demandas_por_vendedor[v][d] = demandas[d][v];
        }
    }

    // Leer capacidades capacidades[i]: m valores
    for (int i = 0; i < m; i++)
        file >> capacidades[i];

    if (file.fail() && !file.eof()) {
        throw std::runtime_error("Error al leer el archivo: datos incompletos o malformados.");
    }
}

std::vector<std::vector<double>> GAPInstance::lista_de_ratios_por_deposito() const {
    std::vector<std::vector<double>> ratios = {};
    for (int i = 0; i < m; i++){
        std::vector<double> ratio_i = {};
        for(int j = 0; j < n; j++){
            ratio_i.push_back(costos[i][j] / demandas[i][j]);
        }
        ratios.push_back(ratio_i);
    }
    return ratios;
}