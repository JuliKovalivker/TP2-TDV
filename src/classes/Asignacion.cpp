#include "Asignacion.h"
#include <numeric>
#include <algorithm>
#include <iostream>
#include <fstream>

Asignacion::Asignacion() : _instancia(nullptr) {}

Asignacion::Asignacion(const GAPInstance* inst) :
    _instancia(inst),
    _deposito_por_vendedor(inst->n, -1)  // n -1 (ninguno esta asignado)
{
    for(int j = 0; j < inst->m; j++){
        _asignacion.push_back({});
        _capacidades_remanentes.push_back(inst->capacidades[j]);
    }
    _asignacion.push_back({}); // Agregar deposito fantasma
    costo = 0;
}

void Asignacion::asignar(int d, int v) {
    _asignacion[d].push_back(v);
    _deposito_por_vendedor[v] = d;

    // Si no es el depósito fantasma
    if (d < _instancia->m) {
        _capacidades_remanentes[d] -= _instancia->demandas[d][v];
    }
    costo += costo_de(d, v);
}

void Asignacion::desasignar(int v) {
    int d = deposito_de(v);
    int d_fantasma = depositos();

    if(d != d_fantasma){ // Si no está en el fantasma
        // Borrar v del deposito actual
        auto it = std::find(_asignacion[d].begin(), _asignacion[d].end(), v);
        _asignacion[d].erase(it);

        // Asignar al fantasma
        _asignacion[d_fantasma].push_back(v);

        _deposito_por_vendedor[v] = d_fantasma;

        _capacidades_remanentes[d] -= _instancia->demandas[d][v];

        costo = costo - costo_de(d, v) + costo_de(d_fantasma, v);
    }
}

void Asignacion::print() const {
    for (int i = 0; i < _asignacion.size(); i++) {
        std::cout << "[" << i << "]: ";
        for (int j = 0; j < _asignacion[i].size(); j++) {
            std::cout << _asignacion[i][j];
            if (j < _asignacion[i].size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

void Asignacion::guardar_en_archivo() const{
    std::ofstream archivo("salida.txt");
    for (int i = 0; i < _asignacion.size() -1; ++i){
        const auto& fila = _asignacion[i];
        for (int j = 0; j < fila.size(); ++j){
            archivo << fila[j];
            if (j + 1 < fila.size()) archivo << " ";
        }
        archivo << "\n";
    }
    archivo.close();
}

bool Asignacion::operator==(const Asignacion& otro) const {
    return costo == otro.costo;
}

bool Asignacion::operator<(const Asignacion& otro) const {
    return costo < otro.costo;
}

bool Asignacion::operator>(const Asignacion& otro) const {
    return costo > otro.costo;
}

Asignacion& Asignacion::operator=(const Asignacion& otra) {
    if (this == &otra) return *this; // evitar auto-asignación

    // copiar miembros
    this->costo = otra.costo;
    this->_asignacion = otra._asignacion;
    this->_capacidades_remanentes = otra._capacidades_remanentes;
    this->_deposito_por_vendedor = otra._deposito_por_vendedor;
    this->_instancia = otra._instancia;

    return *this;
}

// Leer sobre la instancia
int Asignacion::vendedores() const{
    return _instancia->n;
}

int Asignacion::depositos() const{
    return _instancia->m;
}

int Asignacion::deposito_de(int vendedor) const{
    return _deposito_por_vendedor[vendedor];
}

const std::vector<int>& Asignacion::vendedores_de(int deposito) const{
    return _asignacion[deposito];
}

int Asignacion::costo_de(int deposito, int vendedor) const {
    if(!es_deposito_fantasma(deposito)){
        return _instancia->costos[deposito][vendedor];
    }
    return _instancia->costo_max*3;
}

int Asignacion::demanda_de(int deposito, int vendedor) const {
    if(es_deposito_fantasma(deposito)){
        return 0;
    }
    return _instancia->demandas[deposito][vendedor];
}

// PRE: No es el deposito fantasma
int Asignacion::capacidad_remanente(int deposito) const{
    return _capacidades_remanentes[deposito];
}

bool Asignacion::es_deposito_fantasma(int deposito) const {
    return deposito == _instancia->m;
}

bool Asignacion::hay_lugar(int d, int v) const {
    if(es_deposito_fantasma(d)) return true;
    return capacidad_remanente(d) >= demanda_de(d,v);
}

bool Asignacion::es_factible_swap(int v1, int v2) const {
    int d1 = deposito_de(v1);
    int d2 = deposito_de(v2);

    if(d1 == d2) return true; // si ya estan en el mismo, siempre puedo swapear

    // si d1 es fantasma
    if(es_deposito_fantasma(d1)) {
        int c2 = capacidad_remanente(d2);
        return c2 - _instancia->demandas[d2][v2] >= _instancia->demandas[d2][v1];
    }
    else if(es_deposito_fantasma(d2)) {
        int c1 = capacidad_remanente(d1);
        return c1 - _instancia->demandas[d1][v1] >= _instancia->demandas[d1][v2];
    } else {
        int c1 = capacidad_remanente(d1);
        int c2 = capacidad_remanente(d2);

        return c1 - _instancia->demandas[d1][v1] >= _instancia->demandas[d1][v2] && c2 - _instancia->demandas[d2][v2] >= _instancia->demandas[d2][v1];
    }
}

// PRE: v1 y v2 contienen 2 vendedores de un mismo depósito.
bool Asignacion::es_factible_2swap(std::pair<int, int> v1, std::pair<int, int> v2) const{
    // del primer deposito
    int v11 = v1.first;
    int v12 = v1.second;

    // del segundo deposito
    int v21 = v2.first;
    int v22 = v2.second;

    int d1 = deposito_de(v11);
    int d2 = deposito_de(v21);

    if(d1 == d2) return true; // si ya estan en el mismo, siempre puedo swapear

    // si d1 es fantasma
    if(es_deposito_fantasma(d1)) {
        int c2 = capacidad_remanente(d2);
        return c2 - _instancia->demandas[d2][v21] - _instancia->demandas[d2][v22] >= _instancia->demandas[d2][v11] + _instancia->demandas[d2][v12];
    }
    else if(es_deposito_fantasma(d2)) {
        int c1 = capacidad_remanente(d1);
        return  c1 - _instancia->demandas[d1][v11] - _instancia->demandas[d1][v12]  >= _instancia->demandas[d1][v21] + _instancia->demandas[d1][v22];
    } else {
        int c1 = capacidad_remanente(d1);
        int c2 = capacidad_remanente(d2);

        return c1 - _instancia->demandas[d1][v11] - _instancia->demandas[d1][v12]  >= _instancia->demandas[d1][v21] + _instancia->demandas[d1][v22] && c2 - _instancia->demandas[d2][v21] - _instancia->demandas[d2][v22] >= _instancia->demandas[d2][v11] + _instancia->demandas[d2][v12];
    }
}

// Mandar v1 al deposito de v2 y viceversa
void Asignacion::swap(int v1, int v2) {
    int d1 = deposito_de(v1);
    int d2 = deposito_de(v2);
    if(d1 != d2) {
        costo = costo - costo_de(d1,v1) - costo_de(d2, v2) + costo_de(d1, v2) + costo_de(d2, v1);
        _deposito_por_vendedor[v1] = d2;
        _deposito_por_vendedor[v2] = d1;

        auto it1 = std::find(_asignacion[d1].begin(), _asignacion[d1].end(), v1);
        auto it2 = std::find(_asignacion[d2].begin(), _asignacion[d2].end(), v2);

        if (it1 != _asignacion[d1].end() && it2 != _asignacion[d2].end()){
            *it1 = v2;
            *it2 = v1;
        }
        if(!es_deposito_fantasma(d1)) {
            _capacidades_remanentes[d1] += _instancia->demandas[d1][v1] -  _instancia->demandas[d1][v2];
        }
        if(!es_deposito_fantasma(d2)){
            _capacidades_remanentes[d2] += _instancia->demandas[d2][v2] -  _instancia->demandas[d2][v1];
        }
    }
}

// Mandar v al deposito d
void Asignacion::relocate(int d, int v) {
    int d_actual = deposito_de(v);
    if(d_actual != d) {
        costo = costo - costo_de(d_actual,v) + costo_de(d, v);
        _deposito_por_vendedor[v] = d;

        // Borrar v del deposito actual
        auto it = std::find(_asignacion[d_actual].begin(), _asignacion[d_actual].end(), v);
        _asignacion[d_actual].erase(it);

        _asignacion[d].push_back(v);

        if(!es_deposito_fantasma(d)) {
            _capacidades_remanentes[d] -=  _instancia->demandas[d][v];
        }
        if(!es_deposito_fantasma(d_actual)){
            _capacidades_remanentes[d_actual] += _instancia->demandas[d_actual][v];
        }
    }
}

// O(m)
int Asignacion::deposito_mas_barato(int v) const {
    int deposito_actual = deposito_de(v);
    int min = 0;
    for(int d = 0; d < depositos(); d++) {
        if (costo_de(min,v) > costo_de(d,v) && d != deposito_actual){
            min = d;
        }
    }
    return min;
}

// Devuelve la posicion del deposito factible con menor valor en en vec 
// Si no hay ninguno factible, devuelve m+1 (deposito fantasma)
int Asignacion::deposito_min_valido(int v, const std::vector<double> & vec) const {
    int deposito_min = -1;
    for(int d = 0; d < depositos(); d++) {
        if(hay_lugar(d, v)) {
            if(deposito_min == -1) deposito_min = d; // Busco un primer factible
            else if(vec[deposito_min] > vec[d]) deposito_min = d;
        }
    }
    if(deposito_min == -1) return depositos();
    return deposito_min;
}

// Devuelve la posicion del vendedor con menor valor en vec, que entra en el deposito. 
// Sino hay ninguno factible, devuelve -1 
int Asignacion::vendedor_min_valido(int d, const std::vector<double> & vec) const {
    int vendedor_min = -1;
    for(int v = 0; v < vec.size(); v++) {
        if(hay_lugar(d, v) && deposito_de(v) == -1) { // Entra y todavia no lo asigne.
            if(vendedor_min == -1) vendedor_min = v; // Busco un primer factible
            else if(vec[vendedor_min] > vec[v]) vendedor_min = v;
        }
    }
    return vendedor_min;
}