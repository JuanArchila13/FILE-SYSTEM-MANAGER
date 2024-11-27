#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <sstream>

using namespace std;

class Metadata {
public:
    int tam;
    bool file;
    std::string path;
    std::string extension;
    std::string permissions;
    std::string createDate;

    Metadata() : tam(0), file(false), path(""), extension(""), permissions("") {
        std::time_t now = std::time(0);
        createDate = std::ctime(&now);  // Ahora asignamos el resultado de 'ctime' a un string.
    }

    Metadata(std::string createDate, std::string extension, bool file, std::string path, std::string permissions, int tam)
        : createDate(createDate), extension(extension), file(file), path(path), permissions(permissions), tam(tam) {}
};

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

class File {
public:
    Metadata metadata;
    string data;

    File() : data("") {}
    File(Metadata metadata, const string &data) : metadata(metadata), data(data) {}

    string getData() const { return data; }
    Metadata getMetadata() const { return metadata; }
};

class DirecTable {
public:
    string name;
    int cluster;
    Metadata metadata;

    DirecTable() : name(""), cluster(-1) {}
    DirecTable(string name, int cluster, Metadata metadata) : name(name), cluster(cluster), metadata(metadata) {}

    string getName() const { return name; }
    int getCluster() const { return cluster; }
    void setCluster(int newCluster) { cluster = newCluster; }
    Metadata getMetadata() const { return metadata; }
    void setMetadata(Metadata newMetadata) { metadata = newMetadata; }
    string toString() const {
        stringstream ss;
        ss << "DirecTable{name='" << name << "', cluster=" << cluster << "}";
        return ss.str();
    }
};

class FileSystemManager {
private:
    map<string, map<string, DirecTable>> mainTables;
    vector<Cluster> mainClusters;
    vector<Cluster> swapClusters;
    int clusterTam;
    int diskSize;

public:
    FileSystemManager() {
        mainTables = map<string, map<string, DirecTable>>();
        mainTables["/"] = map<string, DirecTable>();
        mainTables["/"]["."] = DirecTable(".", -1, Metadata());
        mainClusters = vector<Cluster>();
        diskSize = obtainDiskSize();

        for (int i = 0; i < diskSize; i++)  // Asignar cantidad de mainClusters
            mainClusters.push_back(Cluster());  // Añadir clusters vacíos
        swapClusters = vector<Cluster>();
        for (int i = 0; i < 100; i++)
            swapClusters.push_back(Cluster());  // Añadir clusters vacíos
        clusterTam = 32;
    }

    // Métodos para crear directorios y archivos
    File createDirectory(string path, string permissions) {
        Metadata metadata = Metadata(
            createDate = ctime(&now);  // Esto devuelve un 'char*', así que lo asignamos directamente a un 'string'.
            "",
            false,
            path,
            permissions,
            0
        );
        return File(metadata, "");
    }

    string saveFile(string path, string name, string data, string permissions) {
        if (path.back() != '/')
            return "path no valido";
        if (!existDirectory(path))
            return "path no existente";
        if (existFile(path, name))
            return "archivo ya existente";
        if (this->numClustersRequired(data) > this->availableNumberOfClusters())
            return "sin espacio disponible";
        
        File file = createFile(path, name, data, permissions);
        string remainingData = file.getData();
        int clusterTemp = -1;
        int previousClusterTemp = -1;
        int remaining = remainingData.length();
        DirecTable direcTable;
        direcTable.setMetadata(file.getMetadata());
        direcTable.setName(name);

        while (remainingData.length() > 0) {
            int tamSave = (remaining > clusterTam) ? clusterTam : remaining;
            string saverData = remainingData.substr(0, tamSave);
            remainingData = remainingData.substr(tamSave);
            remaining = remainingData.length();
            clusterTemp = this->getFirstClusterAvailable();
            Cluster cluster(clusterTemp, saverData, -1);
            mainClusters[clusterTemp] = cluster;

            if (previousClusterTemp != -1)
                mainClusters[previousClusterTemp].next = clusterTemp;
            else {
                direcTable.setCluster(clusterTemp);
                this->getTable(path).insert({direcTable.getName(), direcTable});
            }
            previousClusterTemp = clusterTemp;
        }
        return "CREATED";
    }

    string saveDirectory(string path, string name, string permissions) {
        File directory = createDirectory(path, permissions);
        if (path.back() != '/')
            return "path no valido";
        if (name.back() == '/')
            return "nombre no valido, no ingrese /";
        if (!existDirectory(path))
            return "ruta no existente";
        if (existDirectory(path + name))
            return "carpeta ya existente";
        
        mainTables[path + name + "/"] = map<string, DirecTable>();
        getTable(path).insert({name, DirecTable(name, -1, directory.getMetadata())});
        getTable(path + name + "/").insert({".", DirecTable(".", -1, Metadata())});
        getTable(path + name + "/").insert({"..", DirecTable("..", -1, Metadata())});

        cout << "CREATED" << path + name + "/" << endl;
        return "CREATED" + path + name + "/";
    }

    string deleteFile(string pathAndName) {
        string path = pathAndName.substr(0, pathAndName.rfind("/") + 1);
        string name = pathAndName.substr(pathAndName.rfind("/") + 1);
        return deleteFile(path, name);
    }

    string deleteFile(string path, string name) {
        if (!existDirectory(path))
            return "ruta no existente";
        if (!existFile(path, name))
            return "archivo no existente";
        
        map<string, DirecTable> &map = getTable(path);
        DirecTable direcTable = map[name];
        int clusterNumber = direcTable.getCluster();

        while (clusterNumber != -1) {
            Cluster cluster = mainClusters[clusterNumber];
            mainClusters[clusterNumber] = Cluster();  // Borrar cluster
            clusterNumber = cluster.next;
        }

        map.erase(name);
        return "DELETED";
    }

    string copyFile(string path, string name, string newPath) {
        if (path.back() != '/')
            return "path no valido";
        if (!existFile(path, name))
            return "archivo no existente";
        
        File file = getFile(path, name);
        saveFile(newPath, name, file.getData(), file.getMetadata().permissions);
        return "COPIED";
    }

    string copyDirectory(string path, string name, string newPath) {
        if (!existDirectory(newPath)) {
            return "path no existente";
        }

        map<string, DirecTable> direcTableMap = getTable(path + name + "/");
        saveDirectory(newPath, name, "root");

        for (auto &entry : direcTableMap) {
            DirecTable direcTable = entry.second;
            if (direcTable.getName() != "." && direcTable.getName() != "..") {
                if (direcTable.getMetadata().file) {
                    copyFile(path + name + "/", direcTable.getName(), newPath + name + "/");
                } else {
                    saveDirectory(newPath + name + "/", direcTable.getName(), "root");
                    copyDirectory(path + name + "/", direcTable.getName(), newPath + name + "/");
                }
            }
        }

        return "DIR COPIED";
    }

    // Métodos adicionales para manejar directorios y archivos (suponiendo que ya existen)
    bool existDirectory(string path) {
        return mainTables.find(path) != mainTables.end();
    }

    bool existFile(string path, string name) {
        return getTable(path).find(name) != getTable(path).end();
    }

    map<string, DirecTable>& getTable(string path) {
        return mainTables[path];
    }

    File createFile(string path, string name, string data, string permissions) {
        Metadata metadata = Metadata(
            ctime(&time(0)),
            name.substr(name.find_last_of(".")),
            true,
            path,
            permissions,
            data.length()
        );
        return File(metadata, data);
    }

    int numClustersRequired(string data) {
        return (data.length() + clusterTam - 1) / clusterTam;
    }

    int availableNumberOfClusters() {
        int num = 0;
        for (auto &cluster : mainClusters) {
            if (cluster.number == -1) {
                num++;
            }
        }
        return num;
    }

    int getFirstClusterAvailable() {
        for (int i = 0; i < mainClusters.size(); i++) {
            if (mainClusters[i].number == -1) {
                return i;
            }
        }
        return -1;
    }

    File getFile(string path, string name) {
        return File(getTable(path)[name].getMetadata(), "");
    }

    int obtainDiskSize() {
        return 400;
        // return llamado a discos;
    }

    bool execFile(string filePath, string fileName) {
        bool existFile = mainTables[filePath].count(fileName) > 0;
        if (existFile) {
            // llamado a procesos para que cree el PID del proceso
        }
        return existFile;
    }
};
