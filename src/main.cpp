#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

struct GAPInstance {
    int m; // depósitos
    int n; // vendedores
    std::vector<std::vector<double>> costos; // c[i][j]: costo asignar vendedor j a depósito i
    std::vector<std::vector<double>> demandas; // d[i][j]: demanda de del vendedor i
    std::vector<double> capacidades;             // c[i]: capacidad del depósito i
};

GAPInstance leer_archivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }

    GAPInstance inst;
    file >> inst.m >> inst.n;

    // Reservar matrices m x n
    inst.costos.assign(inst.m, std::vector<double>(inst.n));
    inst.demandas.assign(inst.m, std::vector<double>(inst.n));
    inst.capacidades.resize(inst.m);

    // Leer matriz de costos costos[i][j]: m filas, n columnas
    for (int i = 0; i < inst.m; i++)
        for (int j = 0; j < inst.n; j++)
            file >> inst.costos[i][j];

    // Leer matriz de consumos demandas[i][j]: m filas, n columnas
    for (int i = 0; i < inst.m; i++)
        for (int j = 0; j < inst.n; j++)
            file >> inst.demandas[i][j];

    // Leer capacidades capacidades[i]: m valores
    for (int i = 0; i < inst.m; i++)
        file >> inst.capacidades[i];

    if (file.fail() && !file.eof()) {
        throw std::runtime_error("Error al leer el archivo: datos incompletos o malformados.");
    }

    return inst;
}

int main(int argc, char** argv) {
    std::string filename = "instances/gap/gap_a/a05100";
    if (argc > 1) filename = argv[1];

    std::cout << "Reading file: " << filename << std::endl;

    GAPInstance inst = leer_archivo(filename);

    std::cout << "m (depósitos) = " << inst.m << std::endl;
    std::cout << "n (vendedores) = " << inst.n << std::endl;

    // Verificación: primeros y últimos valores de cada estructura
    std::cout << "c[0][0]=" << inst.costos[0][0]
              << "  c[m-1][n-1]=" << inst.costos[inst.m-1][inst.n-1] << std::endl;
    std::cout << "a[0][0]=" << inst.demandas[0][0]
              << "  a[m-1][n-1]=" << inst.demandas[inst.m-1][inst.n-1] << std::endl;
    std::cout << "b[0]="   << inst.capacidades[0]
              << "  b[m-1]=" << inst.capacidades[inst.m-1] << std::endl;

    return 0;
}