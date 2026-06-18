#include <string>
#include <iostream>

#include "classes/GAPInstance.h"
#include "classes/Asignacion.h"
#include "classes/Solver.h"

int main(int argc, char** argv) {
    std::string filename = "instances/gap/gap_a/a05100";
    int heuristica = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--heuristica-1")
            heuristica = 1;
        else if (arg == "--heuristica-2")
            heuristica = 2;
        else if (arg == "--heuristica-3")
            heuristica = 3;
        else if (arg.substr(0, 2) == "--") {
            std::cerr << "Argumento inválido: " << arg << ". Usar --heuristica-1, --heuristica-2 o --heuristica-3." << std::endl;
            return 1;
        } else
            filename = arg;
    }

    std::cout << "Leyendo: " << filename << std::endl;

    GAPInstance inst = GAPInstance(filename);

    std::cout << "m (depósitos) = " << inst.m << std::endl;
    std::cout << "n (vendedores) = " << inst.n << std::endl;

    Solver solver(inst);
    Asignacion asignacion;

    if (heuristica == 1)
        asignacion = solver.solve(Solver::Heuristica::VARIANZAS);
    else if (heuristica == 2)
        asignacion = solver.solve(Solver::Heuristica::DEPOSITOS);
    else if (heuristica == 3)
        asignacion = solver.solve(Solver::Heuristica::DEMANDAS);
    else {
        std::cerr << "Heurística inválida. Usar --heuristica-1, --heuristica-2 o --heuristica-3." << std::endl;
        return 1;
    }

    asignacion.print();
    std::cout << asignacion.costo << std::endl;
    solver.optimizar(asignacion, 3, 10);
    std::cout << asignacion.costo << std::endl;

    asignacion.guardar_en_archivo();
    return 0;
}