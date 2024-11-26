#include "Metadata.h"
#include <ctime>

// Constructor por defecto
Metadata::Metadata()
    : tam(0), file(false), path(""), extension(""), permissions("") {
    time_t now = time(0);
    createDate = ctime(&now);
}

// Constructor parametrizado
Metadata::Metadata(const std::string& createDate, const std::string& extension, bool file,
                   const std::string& path, const std::string& permissions, int tam)
    : createDate(createDate), extension(extension), file(file), path(path),
      permissions(permissions), tam(tam) {}
