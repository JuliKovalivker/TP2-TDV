#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

// Calcular la varianza para cada vector
double varianza(const std::vector<double>& v);
std::vector<std::pair<int, double>> calcular_varianzas(const std::vector<std::vector<double>> & vec);

// Calcular el valor promedio para cada vector
double promedio(const std::vector<double> & v);
std::vector<std::pair<int, double>> calcular_promedios(const std::vector<std::vector<double>> & vec);

// Ordenar un vector de pairs usando el segundo elemento.
void ordenar_pairs(std::vector<std::pair<int, double>>& vec);
void mergesort(std::vector<std::pair<int, double>>& v, int left, int right);
void merge(std::vector<std::pair<int, double>>& v, int left, int mid, int right);

#endif