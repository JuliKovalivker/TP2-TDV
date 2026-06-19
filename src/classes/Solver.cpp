#include "Solver.h"
#include <stdexcept>
#include <random>
#include "../utils/utils.h"

Solver::Solver(const GAPInstance& inst) : _instancia(&inst) {}

Asignacion Solver::solve(Heuristica h) {
    Asignacion res;
    switch (h) {
        case Heuristica::VARIANZAS: res = heuristica_varianzas(); break;
        case Heuristica::DEPOSITOS: res = heuristica_depositos(); break;
        case Heuristica::DEMANDAS:  res = heuristica_demandas(); break;
        default: throw std::invalid_argument("Heuristica invalida");
    }
    // Podria ser un metodo aparte que optimice
    // metaheuristica(res, 5); // Por ahora hardcodeado 5
    return res;
}

void Solver::optimizar(Asignacion & asig, int k = 5, int CANT_ITERS = 10) {
    metaheuristica(asig, k, CANT_ITERS);
}

/////////////////////////////////////////////////////////
////////////////////// HEURÍSTICAS //////////////////////
/////////////////////////////////////////////////////////

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
    
    std::vector<std::vector<double>> costos = _instancia->costos;

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
            int v = asig.vendedor_min_valido(d, costos[d]);
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

////////////////////////////////////////////////////////////
////////////////////// METAHEURÍSTICA //////////////////////
////////////////////////////////////////////////////////////

Asignacion perturbar_y_copiar(Asignacion & asig, int k, int SEMILLA) {
    Asignacion copia_asig = asig;

    // Elegir k vendedores aleatorios
    std::mt19937 gen(SEMILLA);
    std::uniform_int_distribution<int> dist(0, asig.vendedores()-1);

    for (int i = 0; i < k; i++) {
        int v_a_sacar = dist(gen);
        copia_asig.desasignar(v_a_sacar);
    }
    return copia_asig;
}

void Solver::localSearch(Asignacion & asig, int MAX_ITERS) {
    bool agotado = false;
    for(int j = 0; j < MAX_ITERS && !agotado; j++){
        int c1 = asig.costo;
        dos_swap_aux(asig);
        swap_aux(asig);
        relocate_aux(asig);
        if(asig.costo == c1) agotado = true;
    }
}

void Solver::metaheuristica(Asignacion & asig, int k, int CANT_ITERS){
    const int SEMILLA = 42;
    int k_vendedores = (asig.vendedores() * k / (100));
    Asignacion actual = asig;
    Asignacion best = asig;
    for(int i = 0; i < CANT_ITERS; i++){
        int MAX_ITERS = 100;
        localSearch(actual, MAX_ITERS);
        if(actual.costo < best.costo){
            best = actual;
        }
        actual = perturbar_y_copiar(best, k, SEMILLA + i);
    }
    asig = best;
}

////////////////////////////////////////////////////////////
////////////////////// BÚSQUEDA LOCAL //////////////////////
////////////////////////////////////////////////////////////

template <typename Func>
void Solver::agotar_busqueda_local(Asignacion & asig, Func buqueda_local, int k){
    int c1 = 0;
    int c2 = 1;
    for (int i = 0; i < k && c1 != c2; i++){
        c1 = asig.costo;
        buqueda_local(asig);
        c2 = asig.costo;
    }
}

void Solver::swap_aux(Asignacion & asig){
    int best_costo = asig.costo;
    int best_v1 = -1;
    int best_v2 = -1;
    for(int v1 = 0; v1 < asig.vendedores(); v1++){
        int d1 = asig.deposito_de(v1);
        for(int v2 = v1+1; v2 < asig.vendedores(); v2++){
            int d2 = asig.deposito_de(v2);
            if(d1 == d2) continue;
            if(asig.es_factible_swap(v1,v2)){
                int costo_nuevo = asig.costo - asig.costo_de(d1,v1) - asig.costo_de(d2, v2) + asig.costo_de(d2, v1) + asig.costo_de(d1, v2);
                if(costo_nuevo < best_costo) {
                    best_costo = costo_nuevo;
                    best_v1 = v1;
                    best_v2 = v2;
                }
            }
        }
    }
    if(best_costo < asig.costo) {
        asig.swap(best_v1,best_v2);
    }
}

void Solver::swap(Asignacion & asig){
    agotar_busqueda_local(asig, [this](Asignacion& a){ swap_aux(a); });
}

// 2-swap mas cercanos
void Solver::dos_swap_aux(Asignacion & asig){
    int best_costo = asig.costo;
    std::vector<int> best = {-1,-1,-1,-1}; 
    for(int d1 = 0; d1 <= asig.depositos(); d1++){
        for(int d2 = d1+1; d2 <= asig.depositos(); d2++) {
            std::vector<int> lista_de_vendedores_1 = asig.vendedores_de(d1);
            std::vector<int> lista_de_vendedores_2 = asig.vendedores_de(d2);

            if(lista_de_vendedores_1.size() <= 1 || lista_de_vendedores_2.size() <= 1) continue;

            std::pair<int,int> mejores_1 = dos_mas_baratos(lista_de_vendedores_1, d2, asig);
            std::pair<int,int> mejores_2 = dos_mas_baratos(lista_de_vendedores_2, d1, asig);

            if(asig.es_factible_2swap(mejores_1, mejores_2)){
                // del primer deposito
                int v11 = mejores_1.first;
                int v12 = mejores_1.second;
                
                // del segundo deposito
                int v21 = mejores_2.first;
                int v22 = mejores_2.second;
                int costo_nuevo = asig.costo - asig.costo_de(d1, v11) - asig.costo_de(d1, v12) - asig.costo_de(d2, v21) - asig.costo_de(d2, v22) + asig.costo_de(d1, v21) + asig.costo_de(d1, v22) + asig.costo_de(d2, v11) + asig.costo_de(d2, v12);
                if(costo_nuevo < best_costo){
                    best[0] = v11;
                    best[1] = v21;
                    best[2] = v12;
                    best[3] = v22;
                    best_costo = costo_nuevo;
                }
            }
        }
    }
    if(best_costo < asig.costo){
        asig.swap(best[0],best[1]);
        asig.swap(best[2],best[3]);
    } 
}

void Solver::dos_swap(Asignacion & asig){
    agotar_busqueda_local(asig, [this](Asignacion& a){ dos_swap_aux(a); });
}

void Solver::relocate_aux(Asignacion & asig) {
    int best_costo = asig.costo;
    std::vector<int> best = {-1,-1}; // best[0] es el deposito y best[1] es el vendedor

    for (int v = 0; v < asig.vendedores(); v++){
        int d_actual = asig.deposito_de(v);
        for(int d = 0; d < asig.depositos(); d++){
            if(d == d_actual) continue;
            if(asig.capacidad_remanente(d) > asig.demanda_de(d, v)){
                int costo_nuevo = asig.costo - asig.costo_de(d_actual, v) + asig.costo_de(d,v);
                if(costo_nuevo < best_costo) {
                    best_costo = costo_nuevo;
                    best[0] = d;
                    best[1] = v;
                }
            }
        }
    }
    if(best_costo < asig.costo){
        asig.relocate(best[0],best[1]);
    } 
}

void Solver::relocate(Asignacion & asig) {
    agotar_busqueda_local(asig, [this](Asignacion& a){ relocate_aux(a); });
}

/////////////////////////////////////////////////////////
////////////////////// AUXILIARES ///////////////////////
/////////////////////////////////////////////////////////

// PRE: lista_de_vendedores.size() > 1
std::pair<int,int> Solver::dos_mas_baratos(std::vector<int> lista_de_vendedores, int deposito, const Asignacion & asig) const {
    std::pair<int,int> res = {lista_de_vendedores[0],0}; // 2 mas baratos
    for(int i = 0; i < lista_de_vendedores.size(); i++){
        int v = lista_de_vendedores[i];
        if(asig.costo_de(deposito, v) < asig.costo_de(deposito, res.first)) {
            res.first = v;
        }
    }
    
    if(res.first != lista_de_vendedores[0]) res.second = lista_de_vendedores[0];
    else res.second = lista_de_vendedores[1];

    for(int i = 0; i < lista_de_vendedores.size(); i++){
        int v = lista_de_vendedores[i];
        if(v == res.first) continue;
        if(asig.costo_de(deposito, v) < asig.costo_de(deposito, res.second)) {
            res.second = v;
        }
    }
    return res;
}