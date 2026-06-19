#ifndef GAPINSTANCE_HPP
#define GAPINSTANCE_HPP

#include <string>
#include <vector>

class GAPInstance {
    public:
        GAPInstance() {}
        GAPInstance(const std::string& filename);

        int m; // depositos
        int n; // vendedores

        double costo_max;

        std::vector<std::vector<double>> costos;                // c[d][v]: costo asignar al depósito d el vendedor v
        std::vector<std::vector<double>> costos_por_vendedor;   // c[v][d] costo de asignar el vendedor v al depósito d

        std::vector<std::vector<double>> demandas;              // d[d][v]: demanda de depósito d del vendedor v
        std::vector<std::vector<double>> demandas_por_vendedor; // d[v][d] demanda del vendedor v al depósito d

        std::vector<double> capacidades;                        // c[d]: capacidad del depósito d

        // Armar lista de ratios por vendedor. (ratios[d][v] es el ratio c/u del vendedor v a ir al deposito d)
        std::vector<std::vector<double>> lista_de_ratios_por_deposito() const;
    private:
        void leer_archivo(const std::string& filename);
};

#endif