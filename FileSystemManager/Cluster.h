#ifndef CLUSTER_H
#define CLUSTER_H

#include <string>
#include <sstream>

class Cluster {
public:
    // Miembros de datos
    int number;
    std::string data;
    int next;

    // Constructores
    Cluster();  // Constructor por defecto
    Cluster(int number, const std::string& data, int next);

    // Métodos
    std::string toString() const;  // Convierte el objeto a una cadena representativa
};

#endif // CLUSTER_H
