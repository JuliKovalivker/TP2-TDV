#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <stdexcept>
#include "classes/GAPInstance.h"
#include "classes/Asignacion.h"
#include "utils/utils.h"

const int SEMILLA = 20;


///////////////////////////// HEURISTICA 1 ///////////////////////////

// Devuelve la posicion del deposito con menor ratio c/u
// Sino hay ninguno factible, devuelve m+1 (deposito fantasma)
int min_valido(const int sig, const std::vector<double> & costos, const Asignacion & asig){
    int deposito_min = -1;
    for(int d = 0; d < asig.depositos(); d++) {
        if(asig.hay_lugar(d, sig)) {
            if(deposito_min == -1) deposito_min = d; // Busco un primer factible
            else if(costos[deposito_min] > costos[d]) deposito_min = d;
        }
    }
    if(deposito_min == -1) return asig.depositos();
    return deposito_min;
}

Asignacion heuristica_1(GAPInstance & inst) {
    Asignacion asignacion = Asignacion(inst);

    std::vector<std::vector<double>> costos = inst.costos_por_vendedor;

    std::vector<std::pair<int, double>> varianzas = calcular_varianzas(costos);
    ordenar_pairs(varianzas);

    int asignados = 0;
    while (asignados < inst.n) {
        std::pair<int, double> sig = varianzas[varianzas.size() - asignados - 1]; // Asginamos el de mayor varianza
        int vendedor = sig.first;
        int deposito = min_valido(vendedor, costos[vendedor], asignacion);
        asignacion.asignar(deposito, vendedor);
        
        // si lo sature al deposito -> recalculo varianzas
        // Podriamos ver el min tambien
        //     if(inst.capacidades[deposito] == 0) { TIENE SENTIDO???
        //         recalcular_varianzas(varianazs)
        //     }

        asignados++;
    }
    
    return asignacion;
}

///////////////////////////// HEURISTICA 2 ///////////////////////////

// Armar lista de ratios por vendedor. (ratios[i][j] es el ratio c/u del vendedor i a ir al deposito j)
std::vector<std::vector<double>> lista_de_ratios_por_deposito(const GAPInstance & inst) {
    std::vector<std::vector<double>> ratios = {};
    for (int i = 0; i < inst.m; i++){
        std::vector<double> ratio_i = {};
        for(int j = 0; j < inst.n; j++){
            ratio_i.push_back(inst.costos[i][j] / inst.demandas[i][j]);
        }
        ratios.push_back(ratio_i);
    }
    return ratios;
}

// Devuelve la posicion del vendedor con menor ratio c/u
// Sino hay ninguno factible, devuelve -1 
int mejor_valido(const std::vector<double> & ratios, const int deposito, const Asignacion & asig, const std::vector<bool> & vendedores_ocupados){
    int vendedor_min = -1;
    for(int v = 0; v < ratios.size(); v++) {
        if(asig.hay_lugar(deposito, v) && !vendedores_ocupados[v]) {
            if(vendedor_min == -1) vendedor_min = v; // Busco un primer factible
            else if(ratios[vendedor_min] > ratios[v]) vendedor_min = v;
        }
    }
    return vendedor_min;
}


Asignacion heuristica_2(GAPInstance & inst) { // PRIMERO DEPOSITOS
    Asignacion asignacion = Asignacion(inst);
    
    std::vector<std::vector<double>> ratios = lista_de_ratios_por_deposito(inst);

    // Depositos ordenados por capacidad (menor a mayor)
    std::vector<std::pair<int, double>> depositos = {};
    for(int i = 0; i < inst.m; i++){
        depositos.push_back(std::pair(i,inst.capacidades[i]));
    }
    ordenar_pairs(depositos);

    std::vector<bool> vendedores_ocupados(inst.n, false);
    int asignados = 0;
    bool cambie = true;

    while (cambie && asignados != inst.n) {
        cambie = false;
        for(int i = 0; i < inst.m; i++) {
            int d = depositos[i].first; // el siguiente deposito
            int v = mejor_valido(ratios[d], d, asignacion, vendedores_ocupados);
            if(v != -1) {
                asignacion.asignar(d,v);
                inst.capacidades[d] -= inst.demandas[d][v];
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
        asignacion.asignar(inst.m, i);
    }
    return asignacion;
}

/////////////////// HEURISTICA 3 ////////////////////////

Asignacion heuristica_3(GAPInstance & inst) {
    Asignacion asignacion = Asignacion(inst);

    std::vector<std::vector<double>> demandas = inst.demandas_por_vendedor;

    std::vector<std::pair<int, double>> promedios = calcular_promedios(demandas);
    ordenar_pairs(promedios);
   
    int asignados = 0;
    while (asignados < inst.n) {
        std::pair<int, double> sig = promedios[promedios.size() - asignados - 1]; // Asginamos el que tiene mayor demanda en promedio
        int vendedor = sig.first;
        int deposito = min_valido(vendedor, demandas[vendedor], asignacion);
        asignacion.asignar(deposito, vendedor);

        asignados++;
    }
    
    return asignacion;
}

///////////////// BUSQUEDA LOCAL SWAP CORRIENDO SOLAMENTE CON EL MEJOR DEPOSITO ///////////////
void busqueda_local_1bis(Asignacion & asig) {
    for(int v1 = 0; v1 < asig.vendedores(); v1++){
        int d1 = asig.deposito_de(v1);
        int d2 = asig.deposito_mas_barato(v1);

        std::vector<int> lista_de_vendedores = asig.vendedores_de(d2);

        for(int i = 0; i < lista_de_vendedores.size(); i++) {
            int v2 = lista_de_vendedores[i];
            if(v1 != v2 && d1 != d2 && asig.es_factible_swap(v1,v2)){
                int costo_nuevo = asig.costo - asig.costo_de(d1,v1) - asig.costo_de(d2, v2) + asig.costo_de(d2, v1) + asig.costo_de(d1, v2);
                if(costo_nuevo < asig.costo) {
                    asig.swap(v1,v2);
                }
            }
        }
    }
}
///////////////// BUSQUEDA LOCAL SWAP ///////////////
void busqueda_local_1_aux(Asignacion & asig){
    for(int v1 = 0; v1 < asig.vendedores(); v1++){
        int d1 = asig.deposito_de(v1);

        for(int d2 = 0; d2 <= asig.depositos(); d2++) { // incluye depósito "fantasma"
            if(d1 == d2) continue;
            std::vector<int> lista_de_vendedores = asig.vendedores_de(d2);
    
            for(int i = 0; i < lista_de_vendedores.size(); i++) {
                int v2 = lista_de_vendedores[i];
                if(v1 != v2 && asig.es_factible_swap(v1,v2)){
                    int costo_nuevo = asig.costo - asig.costo_de(d1,v1) - asig.costo_de(d2, v2) + asig.costo_de(d2, v1) + asig.costo_de(d1, v2);
                    if(costo_nuevo < asig.costo) {
                        asig.swap(v1,v2);
                    }
                }
            }
        }
    }
}

void busqueda_local_1(Asignacion & asig, int k = 100) {
    int c1 = 0;
    int c2 = 1;
    for (int i = 0; i < k && c1 != c2; i++){
        c1 = asig.costo;
        busqueda_local_1_aux(asig);
        c2 = asig.costo;
    }
}



///////////////// BUSQUEDA LOCAL 2-SWAP MAS CERCANOS ///////////////

// PRE: lista_de_vendedores.size() > 1
std::pair<int,int> dos_mas_baratos(std::vector<int> lista_de_vendedores, int deposito, const Asignacion & asig) {
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

void busqueda_local_2_aux(Asignacion & asig){
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

void busqueda_local_2(Asignacion & asig, int k = 100) {
    int c1 = 0;
    int c2 = 1;
    for (int i = 0; i < k && c1 != c2; i++){
        c1 = asig.costo;
        busqueda_local_2_aux(asig);
        c2 = asig.costo;
    }
}

///////////////// BUSQUEDA LOCAL RELOCATE ///////////////

void busqueda_local_3_aux(Asignacion & asig){
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

void busqueda_local_3(Asignacion & asig, int k = 100) {
    int c1 = 0;
    int c2 = 1;
    for (int i = 0; i < k && c1 != c2; i++){
        c1 = asig.costo;
        busqueda_local_3_aux(asig);
        c2 = asig.costo;
    }
}

///////////////// METAHEURÍSTICA ///////////////

// Perturbar la asignacion sacando k vendedores random
void metaheuristica_1(Asignacion & asig, int k){
    Asignacion copia_asig = asig;

    // Elegir k vendedores aleatorios
    std::mt19937 gen(SEMILLA);
    std::uniform_int_distribution<int> dist(0, asig.vendedores()-1);

    for (int i = 0; i < k; i++) {
        int v_a_sacar = dist(gen);
        copia_asig.desasignar(v_a_sacar);
    }

    busqueda_local_3(copia_asig);
    busqueda_local_1(copia_asig);
    busqueda_local_3(copia_asig);

    if(copia_asig.costo < asig.costo){
        asig = copia_asig;
    }
}

int main(int argc, char** argv) {
    std::string filename = "instances/gap/gap_a/a05100";
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
            std::cerr << "Argumento inválido: " << arg << ". Usar --heuristica-1, --heuristica-2 o --heuristica-3." << std::endl;
            return 1;
        } else
            filename = arg;
    }

    std::cout << "Leyendo: " << filename << std::endl;

    GAPInstance inst = GAPInstance(filename);

    std::cout << "m (depósitos) = " << inst.m << std::endl;
    std::cout << "n (vendedores) = " << inst.n << std::endl;

    Asignacion asignacion;

    if (heuristica == 1)
        asignacion = heuristica_1(inst);
    else if (heuristica == 2)
        asignacion = heuristica_2(inst);
    else if (heuristica == 3)
        asignacion = heuristica_3(inst);
    else {
        std::cerr << "Heurística inválida. Usar --heuristica-1, --heuristica-2 o --heuristica-3." << std::endl;
        return 1;
    }

    asignacion.print();
    std::cout << asignacion.costo << std::endl;

    busqueda_local_3(asignacion);
    metaheuristica_1(asignacion, 10);

    std::cout << asignacion.costo << std::endl;
    return 0;
}