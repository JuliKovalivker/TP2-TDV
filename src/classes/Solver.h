#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "GAPInstance.h"
#include "Asignacion.h"

class Solver {
    public:
        Solver(GAPInstance& inst);

        enum class Heuristica { VARIANZAS, DEPOSITOS, DEMANDAS };

        Asignacion solve(Heuristica h);

    private:
        GAPInstance* _instancia;

        // Heuristicas
        Asignacion heuristica_varianzas();
        Asignacion heuristica_depositos();
        Asignacion heuristica_demandas();

        // Buqueda local
        template <typename Func>
        void agotar_busqueda_local(Asignacion & asig, Func buqueda_local, int k = 100);

        void swap(Asignacion & asig);
        void dos_swap(Asignacion & asig);
        void relocate(Asignacion & asig);
};

#endif