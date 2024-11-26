#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <ctime>

class Metadata {
public:
    // Miembros de datos
    int tam;
    bool file;
    std::string path;
    std::string extension;
    std::string permissions;
    std::string createDate;

    // Constructores
    Metadata();  // Constructor por defecto
    Metadata(const std::string& createDate, const std::string& extension, bool file,
             const std::string& path, const std::string& permissions, int tam);

    // Métodos adicionales pueden ir aquí si los necesitas en la clase.
};

#endif // METADATA_H
