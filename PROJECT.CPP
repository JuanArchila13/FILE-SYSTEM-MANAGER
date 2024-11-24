#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Leer un sector del disco (por ejemplo, 512 bytes)
bool readSector(const string& diskPath, size_t sectorNumber, vector<char>& buffer) {
    ifstream disk(diskPath, ios::binary);
    if (!disk) {
        cerr << "Error al abrir el archivo del disco." << endl;
        return false;
    }

    // Cada sector tiene 512 bytes (puede variar dependiendo del disco)
    size_t sectorSize = 512;
    disk.seekg(sectorNumber * sectorSize, ios::beg);  // Desplazarse al sector adecuado
    disk.read(buffer.data(), sectorSize);

    return disk.good();
}

int main() {
    // Ruta al archivo de imagen de disco (o dispositivo de almacenamiento)
    string diskPath = "disco.img";
    
    // Leer el sector 0 (esto es típicamente donde se encuentra el BPB)
    vector<char> sectorBuffer(512);  // Buffer de 512 bytes para leer el sector
    if (readSector(diskPath, 0, sectorBuffer)) {
        cout << "Sector 0 leído correctamente." << endl;
        // Aquí podrías analizar el BPB o los datos que necesites
    } else {
        cout << "Error al leer el sector." << endl;
    }

    return 0;
}
