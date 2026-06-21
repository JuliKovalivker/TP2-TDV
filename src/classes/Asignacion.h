#ifndef ASIGNACION_HPP
#define ASIGNACION_HPP

#include <vector>
#include <utility>
#include "GAPInstance.h"

class Asignacion {
    public:
        // CONSTRUCTORES
        Asignacion();
        Asignacion(const GAPInstance* inst);    // O(M)

        double costo;

        // MODIFICADORES
        void asignar(int d, int v);     // O(1)
        void desasignar(int v);         // O(N)
        void swap(int v1, int v2);      // O(N)
        void relocate(int d, int v);    // O(N)

        // OPERADORES
        bool operator==(const Asignacion& otro) const;  // O(1)
        bool operator<(const Asignacion& otro) const;   // O(1)
        bool operator>(const Asignacion& otro) const;   // O(1)
        Asignacion& operator=(const Asignacion& otra);  // O(N*M)

        // OBSERVADORES
        int vendedores() const; // O(1)
        int depositos() const;  // O(1)

        int deposito_de(int vendedor) const;                                            // O(1)
        const std::vector<int>& vendedores_de(int deposito) const;                      // O(1)
        double costo_de(int deposito, int vendedor) const;                              // O(1)
        double demanda_de(int deposito, int vendedor) const;                            // O(1)

        bool es_deposito_fantasma(int deposito) const;                                  // O(1)
        
        double capacidad_remanente(int deposito) const;                                 // O(1)
        bool hay_lugar(int d, int v) const;                                             // O(1)
        bool es_factible_swap(int v1, int v2) const;                                    // O(1)
        bool es_factible_2swap(std::pair<int, int> v1, std::pair<int, int> v2) const;   // O(1)

        int deposito_mas_barato(int v) const;                                           // O(M)
        int deposito_min_valido(int v, const std::vector<double> & vec) const;          // O(N)
        int vendedor_min_valido(int d, const std::vector<double> & vec) const;          // O(N)

        void print() const;                 // O(N*M)
        void guardar_en_archivo() const;    // O(N*M)

    private:
        const GAPInstance* _instancia;
        std::vector<std::vector<int>> _asignacion;
        std::vector<int> _deposito_por_vendedor;
        std::vector<float> _capacidades_remanentes;
};

#endif