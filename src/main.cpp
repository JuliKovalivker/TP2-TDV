#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <numeric>
#include <stdexcept>

struct GAPInstance {
    int m; // depósitos
    int n; // vendedores
    std::vector<std::vector<double>> costos; // c[i][j]: costo asignar al depósito i el vendedor j 
    std::vector<std::vector<double>> demandas; // d[i][j]: demanda de depósito i del vendedor j  
    std::vector<double> capacidades;             // c[i]: capacidad del depósito j

    int costo_max = 0; // costo maximo de la instancia
};

GAPInstance leer_archivo(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }

    GAPInstance inst;
    file >> inst.m >> inst.n;

    // Reservar matrices m x n
    inst.costos.assign(inst.m, std::vector<double>(inst.n));
    inst.demandas.assign(inst.m, std::vector<double>(inst.n));
    inst.capacidades.resize(inst.m);
    
    // Leer matriz de costos costos[i][j]: m filas, n columnas
    for (int i = 0; i < inst.m; i++)
        for (int j = 0; j < inst.n; j++) {
            file >> inst.costos[i][j];
            if (inst.costos[i][j] > inst.costo_max)
                inst.costo_max = inst.costos[i][j];
        }
        
    // Leer matriz de consumos demandas[i][j]: m filas, n columnas
    for (int i = 0; i < inst.m; i++)
        for (int j = 0; j < inst.n; j++)
            file >> inst.demandas[i][j];

    // Leer capacidades capacidades[i]: m valores
    for (int i = 0; i < inst.m; i++)
        file >> inst.capacidades[i];

    if (file.fail() && !file.eof()) {
        throw std::runtime_error("Error al leer el archivo: datos incompletos o malformados.");
    }

    return inst;
}

///////////////////////////// HEURISTICA 1 ///////////////////////////

// Armar lista de ratios por vendedor. (ratios[i][j] es el ratio c/u del vendedor i a ir al deposito j)
std::vector<std::vector<float>> lista_de_ratios(const GAPInstance & inst) {
    std::vector<std::vector<float>> ratios = {};
    for (int j = 0; j < inst.n; j++){
        std::vector<float> ratio_j = {};
        for(int i = 0; i < inst.m; i++){
            ratio_j.push_back(inst.costos[i][j] / inst.demandas[i][j]);
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

std::vector<std::vector<int>> heuristica_1(GAPInstance & inst) {
    std::vector<std::vector<int>> asignacion = {};
    for(int j = 0; j <= inst.m; j++){ // m+1 depositos. Los vendedores asignados al m+1, no van a ningun deposito.
        asignacion.push_back({});
    }
    std::vector<std::vector<float>> ratios = lista_de_ratios(inst);

    std::vector<std::pair<int, float>> varianzas = calcular_varianzas(ratios);
    ordenar_pairs(varianzas);

    int asignados = 0;
    while (asignados < inst.n) {
        std::pair<int, float> sig = varianzas[varianzas.size() - asignados - 1]; // Asginamos el de mayor varianza
        int vendedor = sig.first;
        int deposito = min_valido(vendedor, ratios[vendedor], inst);
        asignacion[deposito].push_back(vendedor);

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


std::vector<std::vector<int>> heuristica_2(GAPInstance & inst) { // PRIMERO DEPOSITOS
    std::vector<std::vector<int>> asignacion = {};
    for(int j = 0; j < inst.m; j++){ // m+1 depositos. Los vendedores asignados al m+1, no van a ningun deposito.
        asignacion.push_back({});
    }
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
                asignacion[d].push_back(v);
                inst.capacidades[d] -= inst.demandas[d][v];
                vendedores_ocupados[v] = true;
                asignados++;
                cambie = true;
            }
        }
    }

    std::vector<int> indices_falsos;
    for (int i = 0; i < vendedores_ocupados.size(); i++) {
        if (!vendedores_ocupados[i])
            indices_falsos.push_back(i);
    }
    asignacion.push_back(indices_falsos); // los que quedaron sin asignar
    return asignacion;
}


void print_asignacion(const std::vector<std::vector<int>>& asignacion) {
    for (int i = 0; i < asignacion.size(); i++) {
        std::cout << "[" << i << "]: ";
        for (int j = 0; j < asignacion[i].size(); j++) {
            std::cout << asignacion[i][j];
            if (j < asignacion[i].size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
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
        else if (arg.substr(0, 2) == "--") {
            std::cerr << "Argumento inválido: " << arg << ". Usar --heuristica-1 o --heuristica-2." << std::endl;
            return 1;
        } else
            filename = arg;
    }

    std::cout << "Leyendo: " << filename << std::endl;

    GAPInstance inst = leer_archivo(filename);

    std::cout << "m (depósitos) = " << inst.m << std::endl;
    std::cout << "n (vendedores) = " << inst.n << std::endl;

    std::vector<std::vector<int>> asignacion;

    if (heuristica == 1)
        asignacion = heuristica_1(inst);
    else if (heuristica == 2)
        asignacion = heuristica_2(inst);
    else {
        std::cerr << "Heurística inválida. Usar --heuristica-1 o --heuristica-2" << std::endl;
        return 1;
    }

    print_asignacion(asignacion);
    return 0;
}