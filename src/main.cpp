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

const int SEMILLA = 20;

class GAPInstance {
    public:
        GAPInstance() {}
        GAPInstance(const std::string& filename){leer_archivo(filename);}
        int m; // depositos
        int n; // vendedores

        double costo_max;

        std::vector<std::vector<double>> costos;     // c[i][j]: costo asignar al depósito i el vendedor j 
        std::vector<std::vector<double>> demandas;   // d[i][j]: demanda de depósito i del vendedor j  
        std::vector<double> capacidades;             // c[i]: capacidad del depósito j
    
    private:

        void leer_archivo(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::runtime_error("No se pudo abrir el archivo: " + filename);
            }

            file >> m >> n;

            // Reservar matrices m x n
            costos.assign(m, std::vector<double>(n));
            demandas.assign(m, std::vector<double>(n));
            capacidades.resize(m);
            
            // Leer matriz de costos costos[i][j]: m filas, n columnas
            for (int i = 0; i < m; i++)
                for (int j = 0; j < n; j++) {
                    file >> costos[i][j];
                    if (costos[i][j] > costo_max)
                        costo_max = costos[i][j];
                }
                
            // Leer matriz de consumos demandas[i][j]: m filas, n columnas
            for (int i = 0; i < m; i++)
                for (int j = 0; j < n; j++)
                    file >> demandas[i][j];

            // Leer capacidades capacidades[i]: m valores
            for (int i = 0; i < m; i++)
                file >> capacidades[i];

            if (file.fail() && !file.eof()) {
                throw std::runtime_error("Error al leer el archivo: datos incompletos o malformados.");
            }
        }

};

// Modificar para agregar desde aca
class Asignacion {
    public:
        Asignacion() : _instancia(nullptr) {}
        
        Asignacion(GAPInstance& inst) : 
            _instancia(&inst),
            _deposito_por_vendedor(inst.n, 0)  // n ceros
        {
            for(int j = 0; j < inst.m; j++){
                _asignacion.push_back({});
                _capacidades_remanentes.push_back(inst.capacidades[j]);
            }
            _asignacion.push_back({}); // Agregar deposito fantasma
            costo = 0;
        }
        
        double costo;

        void asignar(int d, int v) {
            _asignacion[d].push_back(v);
            _deposito_por_vendedor[v] = d;

            // Si no es el depósito fantasma
            if (d < _instancia->m) {
                _capacidades_remanentes[d] -= _instancia->demandas[d][v];
            }
            costo += costo_de(d, v);
        }

        void desasignar(int v) {
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

        void print() {
            for (int i = 0; i < _asignacion.size(); i++) {
                std::cout << "[" << i << "]: ";
                for (int j = 0; j < _asignacion[i].size(); j++) {
                    std::cout << _asignacion[i][j];
                    if (j < _asignacion[i].size() - 1) std::cout << ", ";
                }
                std::cout << "\n";
            }
        }


        bool operator==(const Asignacion& otro) const {
            return costo == otro.costo;
        }

        bool operator<(const Asignacion& otro) const {
            return costo < otro.costo;
        }

        bool operator>(const Asignacion& otro) const {
            return costo > otro.costo;
        }

        Asignacion& operator=(const Asignacion& otra) {
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
        int vendedores() {
            return _instancia->n;
        }
        
        int depositos() {
            return _instancia->m;
        }

        int deposito_de(int vendedor) {
            return _deposito_por_vendedor[vendedor];
        }

        std::vector<int>& vendedores_de(int deposito) {
            return _asignacion[deposito];
        }

        int costo_de(int deposito, int vendedor) const {
            if(!es_deposito_fantasma(deposito)){
                return _instancia->costos[deposito][vendedor];
            }
            return _instancia->costo_max*3;
        }

        int demanda_de(int deposito, int vendedor) const {
            if(es_deposito_fantasma(deposito)){
                return 0;
            }
            return _instancia->demandas[deposito][vendedor];
        }

        int capacidad_remanente(int deposito) {
            return _capacidades_remanentes[deposito];
        }

        bool es_deposito_fantasma(int deposito) const {
            return deposito == _instancia->m;
        }

        bool es_factible_swap(int v1, int v2) {
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
        bool es_factible_2swap(std::pair<int, int> v1, std::pair<int, int> v2) {
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
        void swap(int v1, int v2) {
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
        void relocate(int d, int v) {
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
        int deposito_mas_barato(int v){
            int deposito_actual = deposito_de(v);
            int min = 0;
            for(int d = 0; d < depositos(); d++) {
                if (costo_de(min,v) > costo_de(d,v) && d != deposito_actual){
                    min = d;
                }
            }
            return min;
        }

    private:
        GAPInstance* _instancia; 
        std::vector<std::vector<int>> _asignacion;
        std::vector<int> _deposito_por_vendedor;
        std::vector<float> _capacidades_remanentes;
};


///////////////////////////// HEURISTICA 1 ///////////////////////////

// Armar lista de ratios por vendedor. (ratios[i][j] es el ratio c/u del vendedor i a ir al deposito j)
std::vector<std::vector<float>> lista_de_ratios(const GAPInstance & inst) {
    std::vector<std::vector<float>> ratios = {};
    for (int j = 0; j < inst.n; j++){
        std::vector<float> ratio_j = {};
        for(int i = 0; i < inst.m; i++){
            ratio_j.push_back(inst.costos[i][j]); // SI NO MIRAMOS RATIO ES MUCHO MEJOR
        }
        ratios.push_back(ratio_j);
    }
    return ratios;
}

// Function to calculate standard deviation
float varianza(const std::vector<float>& v) {
    if (v.size() <= 1) {
        return 0.0; 
    }

    // 1. Calculate the mean using std::accumulate
    float sum = std::accumulate(v.begin(), v.end(), 0.0);
    float mean = sum / v.size();

    // 2. Accumulate the squared differences from the mean
    float variance_sum = 0.0;
    for (float val : v) {
        variance_sum += (val - mean) * (val - mean);
    }

    float divisor = v.size() - 1;

    // 4. Return the square root of the variance
    return variance_sum / divisor;
}

// Calcular cuanto varía el ratio de cada vendedor
std::vector<std::pair<int, float>> calcular_varianzas(const std::vector<std::vector<float>> & ratios) {
    std::vector<std::pair<int, float>> varianzas = {};
    for(int i = 0; i < ratios.size(); i++) {
        std::pair<int, float> elem = {i, varianza(ratios[i])};
        varianzas.push_back(elem);
    }
    return varianzas;
}


template <typename T>
void merge(std::vector<std::pair<int, T>>& v, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<std::pair<int, T>> L(n1);
    std::vector<std::pair<int, T>> R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = v[left + i];

    for (int i = 0; i < n2; i++)
        R[i] = v[mid + 1 + i];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (L[i].second <= R[j].second) {
            v[k++] = L[i++];
        } else {
            v[k++] = R[j++];
        }
    }

    while (i < n1)
        v[k++] = L[i++];

    while (j < n2)
        v[k++] = R[j++];
}

template <typename T>
void mergesort(std::vector<std::pair<int, T>>& v, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    mergesort(v, left, mid);
    mergesort(v, mid + 1, right);

    merge(v, left, mid, right);
}

template <typename T>
void ordenar_pairs(std::vector<std::pair<int, T>>& vec) {
    if (!vec.empty()) {
        mergesort(vec, 0, vec.size() - 1);
    }
}

bool esFactible(const int vendedor, const int deposito, const GAPInstance & inst){
    return inst.capacidades[deposito] >= inst.demandas[deposito][vendedor];
}

// Devuelve la posicion del deposito con menor ratio c/u
// Sino hay ninguno factible, devuelve m+1 (deposito fantasma)
int min_valido(const int sig, const std::vector<float> & ratios, const GAPInstance & inst){
    int deposito_min = -1;
    for(int d = 0; d < ratios.size(); d++) {
        if(esFactible(sig, d, inst)) {
            if(deposito_min == -1) deposito_min = d; // Busco un primer factible
            else if(ratios[deposito_min] > ratios[d]) deposito_min = d;
        }
    }
    if(deposito_min == -1) return inst.m;
    return deposito_min;
}

Asignacion heuristica_1(GAPInstance & inst) {
    Asignacion asignacion = Asignacion(inst);

    std::vector<std::vector<float>> ratios = lista_de_ratios(inst);

    std::vector<std::pair<int, float>> varianzas = calcular_varianzas(ratios);
    ordenar_pairs(varianzas);

    int asignados = 0;
    while (asignados < inst.n) {
        std::pair<int, float> sig = varianzas[varianzas.size() - asignados - 1]; // Asginamos el de mayor varianza
        int vendedor = sig.first;
        int deposito = min_valido(vendedor, ratios[vendedor], inst);
        asignacion.asignar(deposito, vendedor);

        if(deposito != inst.m) {
            inst.capacidades[deposito] -= inst.demandas[deposito][vendedor];
            
            // si lo sature al deposito -> recalculo varianzas
            // Podriamos ver el min tambien
            //     if(inst.capacidades[deposito] == 0) { TIENE SENTIDO???
            //         recalcular_varianzas(varianazs)
            //     }
        }

        asignados++;
    }
    
    return asignacion;
}

///////////////////////////// HEURISTICA 2 ///////////////////////////

// Armar lista de ratios por vendedor. (ratios[i][j] es el ratio c/u del vendedor i a ir al deposito j)
std::vector<std::vector<float>> lista_de_ratios_por_deposito(const GAPInstance & inst) {
    std::vector<std::vector<float>> ratios = {};
    for (int i = 0; i < inst.m; i++){
        std::vector<float> ratio_i = {};
        for(int j = 0; j < inst.n; j++){
            ratio_i.push_back(inst.costos[i][j] / inst.demandas[i][j]);
        }
        ratios.push_back(ratio_i);
    }
    return ratios;
}

// Devuelve la posicion del vendedor con menor ratio c/u
// Sino hay ninguno factible, devuelve -1 
int mejor_valido(const std::vector<float> & ratios, const int deposito, const GAPInstance & inst, const std::vector<bool> & vendedores_ocupados){
    int vendedor_min = -1;
    for(int v = 0; v < ratios.size(); v++) {
        if(esFactible(v, deposito, inst) && !vendedores_ocupados[v]) {
            if(vendedor_min == -1) vendedor_min = v; // Busco un primer factible
            else if(ratios[vendedor_min] > ratios[v]) vendedor_min = v;
        }
    }
    return vendedor_min;
}


Asignacion heuristica_2(GAPInstance & inst) { // PRIMERO DEPOSITOS
    Asignacion asignacion = Asignacion(inst);
    
    std::vector<std::vector<float>> ratios = lista_de_ratios_por_deposito(inst);

    // Depositos ordenados por capacidad (menor a mayor)
    std::vector<std::pair<int, int>> depositos = {};
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
            int v = mejor_valido(ratios[d], d, inst, vendedores_ocupados);
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

// Armar lista de demandas por vendedor. (demanda[v][d] es el ratio c/u del vendedor v a ir al deposito d)
std::vector<std::vector<float>> lista_de_demandas(const GAPInstance & inst) {
    std::vector<std::vector<float>> demandas = {};
    for (int v = 0; v < inst.n; v++){
        std::vector<float> demanda_v = {};
        for(int d = 0; d < inst.m; d++){
            demanda_v.push_back(inst.demandas[d][v]);
        }
        demandas.push_back(demanda_v);
    }
    return demandas;
}

float promedio(const std::vector<float> & v) {
    return std::accumulate(v.begin(), v.end(), 0.0f) / v.size();
}

std::vector<std::pair<int, float>> calcular_promedios(const std::vector<std::vector<float>> & vec) {
    std::vector<std::pair<int, float>> promedios = {};
    for(int i = 0; i < vec.size(); i++) {
        std::pair<int, float> elem = {i, promedio(vec[i])};
        promedios.push_back(elem);
    }
    return promedios;
}

Asignacion heuristica_3(GAPInstance & inst) {
    Asignacion asignacion = Asignacion(inst);

    std::vector<std::vector<float>> demandas = lista_de_demandas(inst);

    std::vector<std::pair<int, float>> promedios = calcular_promedios(demandas);
    ordenar_pairs(promedios);

    int asignados = 0;
    while (asignados < inst.n) {
        std::pair<int, float> sig = promedios[promedios.size() - asignados - 1]; // Asginamos el que tiene mayor demanda en promedio
        int vendedor = sig.first;
        int deposito = min_valido(vendedor, demandas[vendedor], inst);
        asignacion.asignar(deposito, vendedor);

        if(deposito != inst.m) {
            inst.capacidades[deposito] -= inst.demandas[deposito][vendedor];
            
            // si lo sature al deposito -> recalculo varianzas
            // Podriamos ver el min tambien
            //     if(inst.capacidades[deposito] == 0) { TIENE SENTIDO???
            //         recalcular_varianzas(varianazs)
            //     }
        }

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
void metauristica_1(Asignacion & asig, int k){
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
    metauristica_1(asignacion, 10);

    std::cout << asignacion.costo << std::endl;
    return 0;
}