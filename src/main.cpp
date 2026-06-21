#include <string>
#include <iostream>

#include "classes/GAPInstance.h"
#include "classes/Asignacion.h"
#include "classes/Solver.h"

void mostrar_uso(const std::string& nombre_programa) {
    std::cerr << "Uso: " << nombre_programa << " [archivo_instancia] --heuristica-N" << std::endl;
    std::cerr << "  archivo_instancia   Ruta a la instancia (opcional, por defecto: instances/real/real_instance)" << std::endl;
    std::cerr << "  --heuristica-1      Heurística basada en varianzas" << std::endl;
    std::cerr << "  --heuristica-2      Heurística basada en depósitos" << std::endl;
    std::cerr << "  --heuristica-3      Heurística basada en demandas" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Ejemplo: " << nombre_programa << " instances/real/real_instance --heuristica-1" << std::endl;
}

int main(int argc, char** argv) {
    std::string filename = "instances/real/real_instance";
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
            std::cerr << "Argumento inválido: " << arg << std::endl << std::endl;
            mostrar_uso(argv[0]);
            return 1;
        } else
            filename = arg;
    }

    if (heuristica == 0) {
        std::cerr << "Heurística inválida o no especificada." << std::endl << std::endl;
        mostrar_uso(argv[0]);
        return 1;
    }

    // LEER LA INSTANCIA
    std::cout << "Leyendo: " << filename << std::endl;

    GAPInstance inst = GAPInstance(filename);

    std::cout << "m (depósitos) = " << inst.m << std::endl;
    std::cout << "n (vendedores) = " << inst.n << std::endl;

    // RESOLVERLA con la heuristica solicitada
    Solver solver(inst);
    Asignacion asignacion;

    if (heuristica == 1)
        asignacion = solver.solve(Solver::Heuristica::VARIANZAS);
    else if (heuristica == 2)
        asignacion = solver.solve(Solver::Heuristica::DEPOSITOS);
    else if (heuristica == 3)
        asignacion = solver.solve(Solver::Heuristica::DEMANDAS);

    asignacion.print();
    std::cout << "== COSTO ANTES DE OPTIMIZAR: "<< asignacion.costo << std::endl;
    solver.optimizar(asignacion, 5, 25);
    asignacion.print();
    std::cout << "== COSTO DESPUES DE OPTIMIZAR: "<< asignacion.costo << std::endl;

    asignacion.guardar_en_archivo();
    return 0;
}