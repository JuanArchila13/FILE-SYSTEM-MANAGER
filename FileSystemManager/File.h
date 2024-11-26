#ifndef FILE_H
#define FILE_H

#include <string>
#include "Metadata.h"

class File {
public:
    // Miembros de datos
    Metadata metadata;  // Metadatos del archivo
    std::string data;   // Contenido del archivo

    // Constructores
    File();  // Constructor por defecto
    File(Metadata metadata, const std::string& data);  // Constructor parametrizado
};

#endif // FILE_H
