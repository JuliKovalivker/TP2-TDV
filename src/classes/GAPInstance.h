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

        std::vector<std::vector<double>> costos;     // c[i][j]: costo asignar al depósito i el vendedor j
        std::vector<std::vector<double>> demandas;   // d[i][j]: demanda de depósito i del vendedor j
        std::vector<double> capacidades;             // c[i]: capacidad del depósito j

    private:
        void leer_archivo(const std::string& filename);
};

#endif