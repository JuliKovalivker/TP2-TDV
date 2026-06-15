#ifndef ASIGNACION_HPP
#define ASIGNACION_HPP

#include <vector>
#include <utility>
#include "GAPInstance.h"

class Asignacion {
    public:
        // CONSTRUCTORES
        Asignacion();
        Asignacion(GAPInstance& inst);

        double costo;

        // MODIFICADORES
        void asignar(int d, int v);
        void desasignar(int v);
        void swap(int v1, int v2);
        void relocate(int d, int v);

        // OPERADORES
        bool operator==(const Asignacion& otro) const;
        bool operator<(const Asignacion& otro) const;
        bool operator>(const Asignacion& otro) const;
        Asignacion& operator=(const Asignacion& otra);

        // OBSERVADORES
        int vendedores() const;
        int depositos() const;

        int deposito_de(int vendedor) const;
        const std::vector<int>& vendedores_de(int deposito) const;
        int costo_de(int deposito, int vendedor) const;
        int demanda_de(int deposito, int vendedor) const;

        bool es_deposito_fantasma(int deposito) const;
        
        int capacidad_remanente(int deposito) const;
        bool hay_lugar(int d, int v) const;
        bool es_factible_swap(int v1, int v2) const;
        bool es_factible_2swap(std::pair<int, int> v1, std::pair<int, int> v2) const;

        int deposito_mas_barato(int v) const;

        void print() const;

    private:
        GAPInstance* _instancia;
        std::vector<std::vector<int>> _asignacion;
        std::vector<int> _deposito_por_vendedor;
        std::vector<float> _capacidades_remanentes;
};

#endif