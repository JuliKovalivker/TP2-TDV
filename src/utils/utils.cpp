#include "utils.h"
#include <numeric>

//// VARIANZAS /////
double varianza(const std::vector<double>& v) {
    if (v.size() <= 1) {
        return 0.0; 
    }

    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    double mean = sum / v.size();

    double variance_sum = 0.0;
    for (double val : v) {
        variance_sum += (val - mean) * (val - mean);
    }

    double divisor = v.size() - 1;

    return variance_sum / divisor;
}

std::vector<std::pair<int, double>> calcular_varianzas(const std::vector<std::vector<double>> & vec) {
    std::vector<std::pair<int, double>> varianzas = {};
    for(int i = 0; i < vec.size(); i++) {
        std::pair<int, double> elem = {i, varianza(vec[i])};
        varianzas.push_back(elem);
    }
    return varianzas;
}

//// PROMEDIOS ////

double promedio(const std::vector<double> & v) {
    return std::accumulate(v.begin(), v.end(), 0.0f) / v.size();
}

std::vector<std::pair<int, double>> calcular_promedios(const std::vector<std::vector<double>> & vec) {
    std::vector<std::pair<int, double>> promedios = {};
    for(int i = 0; i < vec.size(); i++) {
        std::pair<int, double> elem = {i, promedio(vec[i])};
        promedios.push_back(elem);
    }
    return promedios;
}

//// ORDENAR ////

void ordenar_pairs(std::vector<std::pair<int, double>>& vec) {
    if (!vec.empty()) {
        mergesort(vec, 0, vec.size() - 1);
    }
}

void mergesort(std::vector<std::pair<int, double>>& v, int left, int right) {
    if (left >= right) return;

    int mid = (right + left) / 2;

    mergesort(v, left, mid);
    mergesort(v, mid + 1, right);

    merge(v, left, mid, right);
}

void merge(std::vector<std::pair<int, double>>& v, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<std::pair<int, double>> L(n1);
    std::vector<std::pair<int, double>> R(n2);

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