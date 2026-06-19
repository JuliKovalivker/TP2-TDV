#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <map>

#include "../../classes/GAPInstance.h"
#include "../../classes/Asignacion.h"
#include "../../classes/Solver.h"

namespace fs = std::filesystem;

int main() {
    const std::vector<std::string> dirs = {
        "instances/gap/gap_a",
        "instances/gap/gap_b",
        "instances/gap/gap_e"
    };

    const std::vector<std::pair<Solver::Heuristica, std::string>> heuristicas = {
        { Solver::Heuristica::VARIANZAS, "heuristica_1" },
        { Solver::Heuristica::DEPOSITOS, "heuristica_2" },
        { Solver::Heuristica::DEMANDAS,  "heuristica_3" }
    };

    // Acumuladores: suma de costo/vendedor y cantidad de instancias válidas por heurística
    std::map<std::string, double> suma_normalizada;
    std::map<std::string, int>    conteo;

    for (const auto& [_, nombre_h] : heuristicas) {
        suma_normalizada[nombre_h] = 0.0;
        conteo[nombre_h]           = 0;
    }

    int total = 0, errores = 0;

    for (const auto& dir : dirs) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "[WARN] Directorio no encontrado: " << dir << std::endl;
            continue;
        }

        std::vector<fs::path> archivos;
        for (const auto& entry : fs::directory_iterator(dir))
            if (entry.is_regular_file())
                archivos.push_back(entry.path());

        for (const auto& filepath : archivos) {
            std::string filename = filepath.string();
            std::string nombre   = filepath.filename().string();

            std::cout << "Procesando: " << filename << std::endl;

            GAPInstance inst = GAPInstance(filename);
            int num_vendedores = inst.n;

            for (const auto& [heuristica, nombre_h] : heuristicas) {
                try {
                    Solver solver(inst);
                    Asignacion asignacion = solver.solve(heuristica);

                    double costo_norm = static_cast<double>(asignacion.costo) / num_vendedores;

                    suma_normalizada[nombre_h] += costo_norm;
                    conteo[nombre_h]++;

                    std::cout << "  " << nombre_h
                              << " -> costo: " << asignacion.costo
                              << " | costo/vendedor: " << costo_norm
                              << std::endl;
                    total++;
                } catch (const std::exception& e) {
                    std::cerr << "  [ERROR] " << nombre_h << " en " << nombre
                              << ": " << e.what() << std::endl;
                    errores++;
                }
            }
        }
    }

    // Imprimir resultado final
    std::cout << "\n===== COSTO/VENDEDOR PROMEDIO POR HEURISTICA =====\n";
    for (const auto& [_, nombre_h] : heuristicas) {
        if (conteo[nombre_h] > 0) {
            double promedio = suma_normalizada[nombre_h] / conteo[nombre_h];
            std::cout << nombre_h << ": " << promedio
                      << "  (sobre " << conteo[nombre_h] << " instancias)\n";
        } else {
            std::cout << nombre_h << ": sin datos válidos\n";
        }
    }

    std::cout << "\nTotal: " << total << " experimentos";
    if (errores > 0) std::cout << ", " << errores << " errores";
    std::cout << std::endl;

    return errores > 0 ? 1 : 0;
}