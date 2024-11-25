#include <iostream>
#include <string>
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
    vector<Cluster> clusters;
    int clusterTam;

public:
    Controller() : clusterTam(32) {
        tables["/"]["."] = DirecTable(".", -1, Metadata());
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
        if (!endsWith(path, "/")) return "Path no válido";
        if (!existDirectory(path)) return "Path no existente";
        if (existFile(path, name)) return "Archivo ya existente";

        File file = createFile(path, name, data, permissions);
        string remainingData = file.data;
        int clusterTemp = -1, previousClusterTemp = -1, remaining = remainingData.length();
        DirecTable direcTable(name, -1, file.metadata);

        while (!remainingData.empty()) {
            int tamSave = min(clusterTam, remaining);
            string saverData = remainingData.substr(0, tamSave);
            remainingData = remainingData.substr(tamSave);
            remaining = remainingData.length();
            clusterTemp = getFirstClusterAvailable();
            Cluster cluster(clusterTemp, saverData, -1);
            if (clusterTemp == clusters.size())
                clusters.push_back(cluster);
            else
                clusters[clusterTemp] = cluster;

            if (previousClusterTemp != -1)
                clusters[previousClusterTemp].next = clusterTemp;
            else {
                direcTable.cluster = clusterTemp;
                tables[path][name] = direcTable;
            }
            previousClusterTemp = clusterTemp;
        }
        return "CREATED";
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
            Cluster cluster = clusters[clusterNumber];
            clusters[clusterNumber] = Cluster();
            clusterNumber = cluster.next;
        }
        map.erase(name);
        return "DELETED";
    }

    string viewClusters() const {
        string res;
        for (const auto &cluster : clusters)
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
            fileData += clusters[clusterNumber].data;
            clusterNumber = clusters[clusterNumber].next;
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
        for (size_t i = 0; i < clusters.size(); ++i)
            if (clusters[i].data.empty())
                return i;
        return clusters.size();
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
