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
#include "classes/Solver.h"
#include "utils/utils.h"

const int SEMILLA = 20;

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

    Solver solver(inst);
    Asignacion asignacion;

    if (heuristica == 1)
        asignacion = solver.solve(Solver::Heuristica::VARIANZAS);
    else if (heuristica == 2)
        asignacion = solver.solve(Solver::Heuristica::DEPOSITOS);
    else if (heuristica == 3)
        asignacion = solver.solve(Solver::Heuristica::DEMANDAS);
    else {
        std::cerr << "Heurística inválida. Usar --heuristica-1, --heuristica-2 o --heuristica-3." << std::endl;
        return 1;
    }

    asignacion.print();
    // std::cout << asignacion.costo << std::endl;

    // busqueda_local_3(asignacion);
    // metaheuristica_1(asignacion, 20);
    // busqueda_local_3(asignacion);

    std::cout << asignacion.costo << std::endl;

    asignacion.guardar_en_archivo();
    return 0;
}