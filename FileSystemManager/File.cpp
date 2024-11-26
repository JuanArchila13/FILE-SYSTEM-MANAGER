#include "File.h"

// Constructor por defecto
File::File() : data("") {}

// Constructor parametrizado
File::File(Metadata metadata, const std::string& data)
    : metadata(metadata), data(data) {}