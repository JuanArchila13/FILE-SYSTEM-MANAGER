#include "Cluster.h"

// Constructor por defecto
Cluster::Cluster() : number(-1), data(""), next(-1) {}

// Constructor parametrizado
Cluster::Cluster(int number, const std::string& data, int next)
    : number(number), data(data), next(next) {}

// Método para representar el objeto como una cadena
std::string Cluster::toString() const {
    std::stringstream ss;
    ss << "N" << number << ": {" << data << "} -> " << next;
    return ss.str();
}
