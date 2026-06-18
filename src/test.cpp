#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include "classes/GAPInstance.h"
#include "classes/Asignacion.h"
#include "classes/Solver.h"

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

    std::ofstream csv("resultados.csv");
    if (!csv.is_open()) {
        std::cerr << "No se pudo abrir resultados.csv para escritura." << std::endl;
        return 1;
    }

    csv << "instancia,heuristica,costo\n";

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

            for (const auto& [heuristica, nombre_h] : heuristicas) {
                try {
                    Solver solver(inst);
                    Asignacion asignacion = solver.solve(heuristica);

                    csv << nombre << "," << nombre_h << "," << asignacion.costo << "\n";
                    csv.flush();

                    std::cout << "  " << nombre_h << " -> costo: " << asignacion.costo << std::endl;
                    total++;
                } catch (const std::exception& e) {
                    std::cerr << "  [ERROR] " << nombre_h << " en " << nombre << ": " << e.what() << std::endl;
                    csv << nombre << "," << nombre_h << ",ERROR\n";
                    csv.flush();
                    errores++;
                }
            }
        }
    }

    csv.close();

    std::cout << "\nListo. " << total << " experimentos completados";
    if (errores > 0) std::cout << ", " << errores << " errores";
    std::cout << ". Resultados en resultados.csv" << std::endl;

    return errores > 0 ? 1 : 0;
}