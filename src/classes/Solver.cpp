#include "Solver.h"
#include <stdexcept>
#include "../utils/utils.h"

#include <iostream>

Solver::Solver(const GAPInstance& inst) : _instancia(&inst) {}

Asignacion Solver::solve(Heuristica h) {
    Asignacion res;
    switch (h) {
        case Heuristica::VARIANZAS: res = heuristica_varianzas(); break;
        case Heuristica::DEPOSITOS: res = heuristica_depositos(); break;
        case Heuristica::DEMANDAS:  res = heuristica_demandas(); break;
        default: throw std::invalid_argument("Heuristica invalida");
    }

    return res;
}

Asignacion Solver::heuristica_varianzas() {
    Asignacion asig = Asignacion(_instancia);

    std::vector<std::vector<double>> costos = _instancia->costos_por_vendedor;

    std::vector<std::pair<int, double>> varianzas = calcular_varianzas(costos);
    ordenar_pairs(varianzas);

    int asignados = 0;
    while (asignados < asig.vendedores()) {
        std::pair<int, double> sig = varianzas[varianzas.size() - asignados - 1]; // Asginamos el de mayor varianza
        int vendedor = sig.first;
        int deposito = asig.deposito_min_valido(vendedor, costos[vendedor]);
        asig.asignar(deposito, vendedor);
        
        // si lo sature al deposito -> recalculo varianzas
        // Podriamos ver el min tambien
        //     if(inst.capacidades[deposito] == 0) { TIENE SENTIDO???
        //         recalcular_varianzas(varianazs)
        //     }

        asignados++;
    }
    
    return asig;
}

Asignacion Solver::heuristica_depositos() {
    Asignacion asig = Asignacion(_instancia);
    
    std::vector<std::vector<double>> ratios = _instancia->lista_de_ratios_por_deposito();
    // std::vector<std::vector<double>> ratios = _instancia->costos_por_vendedor; ///////////////////////////////////////////////////////////////

    // Depositos ordenados por capacidad (menor a mayor)
    std::vector<std::pair<int, double>> depositos = {};
    for(int i = 0; i < asig.depositos(); i++){
        depositos.push_back(std::pair(i,_instancia->capacidades[i]));
    }
    ordenar_pairs(depositos);

    std::vector<bool> vendedores_ocupados(asig.vendedores(), false);
    int asignados = 0;
    bool cambie = true;

    while (cambie && asignados != asig.vendedores()) {
        cambie = false;
        for(int i = 0; i < asig.depositos(); i++) {
            int d = depositos[i].first; // el siguiente deposito
            int v = asig.vendedor_min_valido(d, ratios[d]);
            if(v != -1) {
                asig.asignar(d,v);
                vendedores_ocupados[v] = true;
                asignados++;
                cambie = true;
            }
        }
    }

    // Agregar los que quedaron afuera
    std::vector<int> indices_falsos;
    for (int i = 0; i < vendedores_ocupados.size(); i++) {
        if (!vendedores_ocupados[i])
        asig.asignar(asig.depositos(), i);
    }
    return asig;
}

Asignacion Solver::heuristica_demandas() {
    Asignacion asig = Asignacion(_instancia);

    std::vector<std::vector<double>> demandas = _instancia->demandas_por_vendedor;

    std::vector<std::pair<int, double>> promedios = calcular_promedios(demandas);
    ordenar_pairs(promedios);

    int asignados = 0;
    while (asignados < asig.vendedores()) {
        std::pair<int, double> sig = promedios[promedios.size() - asignados - 1]; // Asginamos el que tiene mayor demanda en promedio
        int vendedor = sig.first;
        int deposito = asig.deposito_min_valido(vendedor, demandas[vendedor]);
        asig.asignar(deposito, vendedor);

        asignados++;
    }
    
    return asig;
}