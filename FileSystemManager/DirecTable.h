#ifndef DIRECTABLE_H
#define DIRECTABLE_H

#include <string>
#include <sstream>
#include "Metadata.h"

class DirecTable {
public:
    // Miembros de datos
    std::string name;   // Nombre del archivo o directorio
    int cluster;        // Número del cluster asociado
    Metadata metadata;  // Metadatos asociados

    // Constructores
    DirecTable();  // Constructor por defecto
    DirecTable(std::string name, int cluster, Metadata metadata);  // Constructor parametrizado

    // Métodos
    std::string toString() const;  // Representación en texto del objeto
};

#endif // DIRECTABLE_H
