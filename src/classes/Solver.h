#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "GAPInstance.h"
#include "Asignacion.h"

class Solver {
    public:
        Solver(const GAPInstance& inst);

        enum class Heuristica { VARIANZAS, DEPOSITOS, DEMANDAS };

        Asignacion solve(Heuristica h);
        
        void optimizar(Asignacion & asig, int k, int CANT_ITERS);
    private:
        const GAPInstance* _instancia;

        // Heuristicas

        Asignacion heuristica_varianzas(); // O(M*N + NlogN)
        Asignacion heuristica_depositos(); // O(M*logM + M*N^2)
        Asignacion heuristica_demandas();  // O(M*N + NlogN)

        // Metaheuristica

        // Perturbar la asignacion sacando k vendedores random
        void metaheuristica(Asignacion & asig, int k, int CANT_ITERS); // O(CANT_ITERS * N * (M+N))
        void localSearch(Asignacion & asig, int MAX_ITERS);            // O(N*M + N^2)

        // Buqueda local que AGOTA
        
        void swap(Asignacion & asig);
        void dos_swap(Asignacion & asig);
        void relocate(Asignacion & asig);
        
        // Auxiliares que ejecutan 1 sola vez la búsqueda
        
        void swap_aux(Asignacion & asig);       // O(N^2)
        void dos_swap_aux(Asignacion & asig);   // O(N^2)
        void relocate_aux(Asignacion & asig);   // O(N*M)
        

        // Auxiliares
        
        template <typename Func>
        void agotar_busqueda_local(Asignacion & asig, Func buqueda_local, int k = 100);
        
        std::pair<int,int> dos_mas_baratos(std::vector<int> lista_de_vendedores, int deposito, const Asignacion & asig) const;
};

#endif