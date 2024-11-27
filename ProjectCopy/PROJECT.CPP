#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

using namespace std;

// =====================
// Clase Metadata
// =====================
class Metadata {
public:
    int tam;
    bool file;
    string path;
    string extension;
    string permissions;
    string createDate;

    Metadata() : tam(0), file(false), path(""), extension(""), permissions("") {
        time_t now = time(0);
        createDate = ctime(&now);
    }

    Metadata(string createDate, string extension, bool file, string path, string permissions, int tam)
        : createDate(createDate), extension(extension), file(file), path(path), permissions(permissions), tam(tam) {}
};

// =====================
// Clase Cluster
// =====================
class Cluster {
public:
    int number;
    string data;
    int next;

    Cluster() : number(-1), data(""), next(-1) {}
    Cluster(int number, const string &data, int next) : number(number), data(data), next(next) {}

    string toString() const {
        stringstream ss;
        ss << "N" << number << ": {" << data << "} -> " << next;
        return ss.str();
    }
};

// =====================
// Clase DirecTable
// =====================
class DirecTable {
public:
    string name;
    int cluster;
    Metadata metadata;

    DirecTable() : name(""), cluster(-1) {}
    DirecTable(string name, int cluster, Metadata metadata) : name(name), cluster(cluster), metadata(metadata) {}
    
    void setMetadata(const Metadata& metadata) {
        this->metadata = metadata;
    }

    void setName(const std::string& name) {
        this->name = name;
    }
    
    string toString() const {
        stringstream ss;
        ss << "DirecTable{name='" << name << "', cluster=" << cluster << "}";
        return ss.str();
    }
};

// =====================
// Clase File
// =====================
class File {
public:
    Metadata metadata;
    string data;

    File() : data("") {}
    File(Metadata metadata, const string &data) : metadata(metadata), data(data) {}
};

// =====================
// Clase Controller
// =====================
class Controller {
private:
    map<string, map<string, DirecTable>> tables;
    vector<Cluster> mainClusters;
    vector<Cluster> swapClusters;
    int clusterTam;
    int diskSize;

public:
    Controller() : clusterTam(32) {
        tables["/"]["."] = DirecTable(".", -1, Metadata());

        // Inicializar mainClusters con `diskSize` elementos nulos
        diskSize = obtainDiskSize();  // Supongo que esta función está definida en otro lugar
        mainClusters.resize(diskSize);  // Rellenar el vector con nullptr

        // Inicializar swapClusters con 100 elementos nulos
        swapClusters.resize(100);  // Rellenar el vector con nullptr
        
    }

    int obtainDiskSize(){
        return 500;
    }

    File createFile(const string &path, const string &name, const string &data, const string &permissions) {
        Metadata metadata = Metadata(getCurrentTime(), getFileExtension(name), true, path, permissions, data.length());
        return File(metadata, data);
    }

    File createDirectory(const string &path, const string &permissions) {
        Metadata metadata = Metadata(getCurrentTime(), "", false, path, permissions, 0);
        return File(metadata, "");
    }

    string saveFile(const string &path, const string &name, const string &data, const string &permissions) {
        cout << "Llego";
        if (!endsWith(path, "/")) return "Path no válido";
        if (!existDirectory(path)) return "Path no existente";
        if (existFile(path, name)) return "Archivo ya existente";
        if (numClustersRequired(data) > availableNumberOfClusters())

        File file = createFile(path, name, data, permissions);  // Define 'file' primero
        string remainingData = data;
        int clusterTemp = -1, previousClusterTemp = -1, remaining = remainingData.length();
        Metadata metadata = Metadata(getCurrentTime(), getFileExtension(name), true, path, permissions, data.length());
        DirecTable direcTable(name, -1, metadata);  // Ahora 'file' ya está definido y puedes acceder a sus métodos
        direcTable.setMetadata(metadata);
        direcTable.setName(name);

        while (remainingData.length() > 0) {
            cout<<"while";
            int tamSave = min(clusterTam, remaining);
            string saverData = remainingData.substr(0, tamSave);
            remainingData = remainingData.substr(tamSave);
            remaining = remainingData.length();
            clusterTemp = getFirstClusterAvailable();
            Cluster cluster(clusterTemp, saverData, -1);
            mainClusters[clusterTemp] = cluster;
            
            cout << previousClusterTemp;
            if (previousClusterTemp != -1){
                cout << "Entro a previus temp" << endl;
                mainClusters[previousClusterTemp].next = clusterTemp;
            }else {
                cout << "No Entro a previus temp" << endl;
                direcTable.cluster = clusterTemp;
                tables[path][name] = direcTable;
            }
            previousClusterTemp = clusterTemp;
        }
        return "CREATED FILE";
    }

    string saveDirectory(const string &path, const string &name, const string &permissions) {
        if (!endsWith(path, "/")) return "Path no válido";
        if (name.find("/") != string::npos) return "Nombre no válido";
        if (!existDirectory(path)) return "Ruta no existente";
        if (existDirectory(path + name + "/")) return "Carpeta ya existente";

        tables[path + name + "/"]["."] = DirecTable(".", -1, Metadata());
        tables[path + name + "/"][".."] = DirecTable("..", -1, Metadata());
        tables[path][name] = DirecTable(name, -1, Metadata(getCurrentTime(), "", false, path + name + "/", permissions, 0));
        return "CREATED";
    }

    string deleteFile(const string &path, const string &name) {
        if (!existDirectory(path)) return "Ruta no existente";
        if (!existFile(path, name)) return "Archivo no existente";

        auto &map = tables[path];
        DirecTable direcTable = map[name];
        int clusterNumber = direcTable.cluster;

        while (clusterNumber != -1) {
            Cluster cluster = mainClusters[clusterNumber];
            mainClusters[clusterNumber] = Cluster();
            clusterNumber = cluster.next;
        }
        map.erase(name);
        return "DELETED";
    }

    string viewClusters() const {
        string res;
        for (const auto &cluster : mainClusters)
            res += cluster.toString() + "\n";
        return res;
    }

    string copyFile(const string &sourcePath, const string &fileName, const string &destPath) {
        if (!existDirectory(sourcePath) || !existFile(sourcePath, fileName)) {
            return "Archivo no existente en la ruta origen.";
        }
        if (!existDirectory(destPath)) {
            return "Ruta destino no existente.";
        }
        if (existFile(destPath, fileName)) {
            return "Archivo ya existente en la ruta destino.";
        }

        DirecTable sourceTable = tables[sourcePath][fileName];
        int clusterNumber = sourceTable.cluster;
        string fileData;

        // Copiar contenido del archivo original
        while (clusterNumber != -1) {
            fileData += mainClusters[clusterNumber].data;
            clusterNumber = mainClusters[clusterNumber].next;
        }

        // Crear el archivo en la ruta destino con los mismos datos
        return saveFile(destPath, fileName, fileData, sourceTable.metadata.permissions);
    }

    string moveFile(const string &sourcePath, const string &fileName, const string &destPath) {
        string copyResult = copyFile(sourcePath, fileName, destPath);
        if (copyResult == "CREATED") {
            return deleteFile(sourcePath, fileName);
        }
        return copyResult; // Si falló la copia, devolver el error.
    }

    void printTables() {
        cout << "=== Estado de tablas ===" << endl;
        for (const auto &dir : tables) {
            cout << "Directorio: " << dir.first << endl;
            for (const auto &entry : dir.second) {
                cout << "  Nombre: " << entry.first
                    << " | Metadata: { Tam: " << entry.second.metadata.tam
                    << ", Path: " << entry.second.metadata.path
                    << ", Permisos: " << entry.second.metadata.permissions
                    << ", Fecha: " << entry.second.metadata.createDate << " }" << endl;
            }
        }
        cout << "========================" << endl;
    }

    int numClustersRequired(const string& data) {
        return static_cast<int>(std::ceil(data.length() / static_cast<double>(clusterTam)));
    }

    string getType(bool isFile) {
        return isFile ? "file" : "directory";
    }

    int availableNumberOfClusters() {
        int num = 0;
        for (size_t i = 0; i < mainClusters.size(); i++) {
            if (mainClusters[i].number == -1)  // Suponiendo que clusterId = -1 indica un "vacío"
                num++;
        }
        return num;
    }

    void init() {
        cout << saveDirectory("/", "root", "root") << endl;
        cout << saveFile("/root/", "archivoa.txt", 
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaa", "root") << endl;
        cout << saveFile("/root/", "archivob.txt", 
                "bbbbbbbbbbbbbbbbbbbbbbbbbb"
                "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                "bbbbbbbbbbbbbbbbbbbbbbbbbb", "root") << endl;
        cout << deleteFile("/root/", "archivoa.txt") << endl;
        cout << saveFile("/root/", "archivoc.txt", 
                "cccccccccccccccccccccccccccccc"
                "cccccccccccccccccccccccccccccc"
                "cccccccccccccccccccccccccccccc"
                "cccccccccccccccccccccccccccccc", "root") << endl;
        cout << saveFile("/root/", "archivod.txt", 
                "ddddddddddddddddddddddddddddddddddddddddd"
                "dddddddddddddddddddddddddddddddddddddddddd"
                "dddddddddddddddddddddddddddddddddddddddddd"
                "dddddddddddddddddddddddddddddddddddddddddd", "root") << endl;
        cout << copyFile("/root/", "archivob.txt", "/") << endl;
        cout << moveFile("/root/", "archivob.txt", "/") << endl;
        cout << viewClusters() << endl;
        printTables();
    }

private:
    bool endsWith(const string &str, const string &suffix) const {
        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    string getFileExtension(const string &name) const {
        size_t pos = name.rfind(".");
        return (pos != string::npos) ? name.substr(pos) : "";
    }

    string getCurrentTime() const {
        time_t now = time(0);
        return ctime(&now);
    }

    int getFirstClusterAvailable() const {
        for (size_t i = 0; i < mainClusters.size(); ++i)
            if (mainClusters[i].data.empty())
                return i;
        return mainClusters.size();
    }

    bool existFile(const string &path, const string &name) const {
        return tables.count(path) && tables.at(path).count(name);
    }

    bool existDirectory(const string &path) const {
        return tables.count(path);
    }
};

// =====================
// Main Function
// =====================
int main() {
    Controller controller;
    controller.init();
    return 0;
}
