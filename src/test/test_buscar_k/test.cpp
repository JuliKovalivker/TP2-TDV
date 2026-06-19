#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <filesystem>
#include <limits>
#include <algorithm>

#include "../../classes/GAPInstance.h"
#include "../../classes/Asignacion.h"
#include "../../classes/Solver.h"

namespace fs = std::filesystem;

std::vector<std::string> listar_instancias(const std::string& dir) {
    std::vector<std::string> instancias;
    if (!fs::exists(dir)) {
        std::cerr << "[WARN] Directorio no encontrado: " << dir << std::endl;
        return instancias;
    }
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file())
            instancias.push_back(entry.path().string());
    }
    std::sort(instancias.begin(), instancias.end());
    return instancias;
}

int main() {
    const std::vector<std::string> dirs = {
        "./instances/gap/gap_a",
        "./instances/gap/gap_b",
        "./instances/gap/gap_e"
    };

    const std::vector<int> ks = {1,2,3,4,5};
    const int CANT_ITER = 25;

    std::vector<std::string> todas_instancias;
    for (const auto& dir : dirs) {
        auto v = listar_instancias(dir);
        todas_instancias.insert(todas_instancias.end(), v.begin(), v.end());
    }

    if (todas_instancias.empty()) {
        std::cerr << "No se encontraron instancias. Verificá los directorios." << std::endl;
        return 1;
    }

    std::cout << "Instancias encontradas: " << todas_instancias.size() << std::endl;

    std::map<int, double> cant_victorias;
    for (int k : ks) cant_victorias[k] = 0.0;

    // Guardamos el costo de cada k en cada instancia para calcular %gap al final
    // historial[i][k] = costo del k en la instancia i
    std::vector<std::map<int, double>> historial;

    int total_instancias_procesadas = 0;

    for (const auto& filepath : todas_instancias) {
        std::cout << "\nProcesando instancia " << total_instancias_procesadas + 1<< "/" << todas_instancias.size() << ": " << filepath << std::endl;

        GAPInstance inst;
        try {
            inst = GAPInstance(filepath);
        } catch (...) {
            std::cerr << "  [ERROR] No se pudo leer la instancia, saltando." << std::endl;
            continue;
        }

        std::map<int, double> costo_por_k;
        
        Solver solver(inst);
        Asignacion inicial = solver.solve(Solver::Heuristica::VARIANZAS);
        Asignacion asignacion = inicial;
        for (int k : ks) {
            try {
                solver.optimizar(asignacion, k, CANT_ITER);
            } catch (...) {
                std::cerr << "  [ERROR] k=" << k << " falló, saltando." << std::endl;
                costo_por_k[k] = std::numeric_limits<double>::max();
                continue;
            }

            costo_por_k[k] = asignacion.costo;
            std::cout << "  k=" << k << "% costo=" << asignacion.costo << std::endl;
            asignacion = inicial;
        }

        // Encontrar el mínimo costo entre todos los ks
        double min_costo = std::numeric_limits<double>::max();
        for (int k : ks)
            if (costo_por_k[k] < min_costo)
                min_costo = costo_por_k[k];

        // Contar cuántos ks empataron en el mínimo
        int empatados = 0;
        for (int k : ks)
            if (costo_por_k[k] == min_costo)
                empatados++;

        // Repartir 1 punto en partes iguales entre los empatados
        for (int k : ks)
            if (costo_por_k[k] == min_costo)
                cant_victorias[k] += 1.0 / empatados;

        historial.push_back(costo_por_k);
        total_instancias_procesadas++;
    }

    // Determinar el k ganador global (mayor cant_victorias)
    int k_ganador = ks[0];
    for (int k : ks)
        if (cant_victorias[k] > cant_victorias[k_ganador])
            k_ganador = k;

    std::cout << "\nK ganador global: " << k_ganador << " con " << cant_victorias[k_ganador] << " puntos" << std::endl;

    // Calcular %gap promedio de cada k respecto al ganador
    // %gap_i = (costo_k_i - costo_ganador_i) / costo_ganador_i * 100
    std::map<int, double> gap_promedio;
    for (int k : ks) gap_promedio[k] = 0.0;

    for (const auto& costo_por_k : historial) {
        double costo_ganador = costo_por_k.at(k_ganador);
        if (costo_ganador <= 0) continue; // evitar division por cero
        for (int k : ks)
            gap_promedio[k] += (costo_por_k.at(k) - costo_ganador) / costo_ganador * 100.0;
    }
    for (int k : ks)
        gap_promedio[k] /= total_instancias_procesadas;

    // Exportar CSV
    const std::string csv_path = "resultados_k.csv";
    std::ofstream csv(csv_path);
    if (!csv.is_open()) {
        std::cerr << "No se pudo crear " << csv_path << std::endl;
        return 1;
    }

    csv << "k,cant_veces_que_gano,%gap\n";
    csv << std::fixed;
    for (int k : ks)
        csv << k << "," << cant_victorias[k] << "," << gap_promedio[k] << "\n";
    csv.close();

    std::cout << "\n=== Resultados ===" << std::endl;
    std::cout << "Instancias procesadas: " << total_instancias_procesadas << std::endl;
    std::cout << "CSV guardado en: " << csv_path << std::endl;
    std::cout << std::fixed;
    std::cout << "\nk\tcant_veces_que_gano\t%gap" << std::endl;
    for (int k : ks)
        std::cout << k << "\t" << cant_victorias[k] << "\t\t\t" << gap_promedio[k] << std::endl;

    return 0;
}