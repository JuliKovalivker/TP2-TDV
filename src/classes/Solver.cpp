#include "Solver.h"

// Solver::Solver(GAPInstance& inst) : _instancia(&inst) {}

// Asignacion Solver::resolver() {
//     Asignacion asign = construir_solucion_inicial();
//     mejorar_solucion(asign);
//     return asign;
// }

// Asignacion Solver::construir_solucion_inicial() {
//     Asignacion asign(*_instancia);

//     // Ejemplo: asignar cada vendedor a su depósito más barato
//     for (int v = 0; v < _instancia->n; v++) {
//         int d = asign.deposito_mas_barato(v);
//         asign.asignar(d, v);
//     }

//     return asign;
// }