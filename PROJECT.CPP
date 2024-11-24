#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

// Estructura para representar una entrada de directorio (32 bytes)
struct DirEntry {
    char filename[11];  // Nombre del archivo (8 + 3)
    uint8_t attributes;  // Atributos (archivo, directorio, oculto, etc.)
    uint16_t firstCluster;  // Primer cluster del archivo
    uint32_t fileSize;  // Tamaño del archivo en bytes

    // Mostrar la entrada del directorio
    void display() {
        string name(filename, 8);
        string ext(filename + 8, 3);
        cout << name << "." << ext << " | ";
        cout << "Cluster: " << firstCluster << " | Size: " << fileSize << " bytes" << endl;
    }
};

// Función para agregar un archivo a un directorio
void addFileToDirectory(vector<DirEntry>& directory, const string& filename, uint16_t firstCluster, uint32_t fileSize) {
    // Crear una nueva entrada de directorio
    DirEntry newEntry;
    memset(&newEntry, 0, sizeof(DirEntry));  // Inicializar a cero

    // Llenar el nombre del archivo (en formato 8.3)
    string name = filename.substr(0, 8);  // Limitar a 8 caracteres
    string ext = filename.substr(8, 3);   // Limitar a 3 caracteres
    memcpy(newEntry.filename, name.c_str(), name.size());
    memcpy(newEntry.filename + 8, ext.c_str(), ext.size());

    // Establecer atributos (0x20 significa archivo normal)
    newEntry.attributes = 0x20;  // Archivo normal

    // Establecer primer cluster y tamaño
    newEntry.firstCluster = firstCluster;
    newEntry.fileSize = fileSize;

    // Agregar la entrada al directorio
    directory.push_back(newEntry);
}

// Mostrar el contenido del directorio
void listDirectory(const vector<DirEntry>& directory) {
    cout << "Directorio:\n";
    for (const auto& entry : directory) {
        entry.display();
    }
}

int main() {
    // Directorio raíz (en memoria, simplificado)
    vector<DirEntry> rootDirectory;

    // Simular la creación de un archivo "MI_ARCHIVO.TXT"
    addFileToDirectory(rootDirectory, "MI_ARCHIVO.TXT", 5, 1024);

    // Listar el contenido del directorio raíz
    listDirectory(rootDirectory);

    return 0;
}
