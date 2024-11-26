#include "DirecTable.h"

// Constructor por defecto
DirecTable::DirecTable() : name(""), cluster(-1) {}

// Constructor parametrizado
DirecTable::DirecTable(std::string name, int cluster, Metadata metadata)
    : name(name), cluster(cluster), metadata(metadata) {}

// Representación en texto del objeto
std::string DirecTable::toString() const {
    std::stringstream ss;
    ss << "DirecTable{name='" << name << "', cluster=" << cluster << "}";
    return ss.str();
}
