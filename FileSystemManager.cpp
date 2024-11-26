#include "FileSystemManager.h"

// =====================
// Implementación Metadata
// =====================
// Metadata::Metadata() : tam(0), file(false), path(""), extension(""), permissions("") {
//     time_t now = time(0);
//     createDate = ctime(&now);
// }

Metadata::Metadata() : extension(""), file(false), path(""),  permissions(""), tam(0) {
    time_t now = time(0);
    createDate = ctime(&now);
}

Metadata::Metadata(string createDate, string extension, bool file, string path, string permissions, int tam)
    : createDate(createDate), extension(extension), file(file), path(path), permissions(permissions), tam(tam) {}

// =====================
// Implementación Cluster
// =====================
Cluster::Cluster() : number(-1), data(""), next(-1) {}

Cluster::Cluster(int number, const string &data, int next) : number(number), data(data), next(next) {}

string Cluster::toString() const {
    stringstream ss;
    ss << "N" << number << ": {" << data << "} -> " << next;
    return ss.str();
}

// =====================
// Implementación DirecTable
// =====================
DirecTable::DirecTable() : name(""), cluster(-1) {}

DirecTable::DirecTable(string name, int cluster, Metadata metadata) : name(name), cluster(cluster), metadata(metadata) {}

string DirecTable::toString() const {
    stringstream ss;
    ss << "DirecTable{name='" << name << "', cluster=" << cluster << "}";
    return ss.str();
}

// =====================
// Implementación File
// =====================
File::File() : data("") {}

File::File(Metadata metadata, const string &data) : metadata(metadata), data(data) {}

// =====================
// Implementación Controller
// =====================
Controller::Controller() : clusterTam(32) {
    tables["/"]["."] = DirecTable(".", -1, Metadata());
}

bool Controller::endsWith(const string &str, const string &suffix) const {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string Controller::getFileExtension(const string &name) const {
    size_t pos = name.rfind(".");
    return (pos != string::npos) ? name.substr(pos) : "";
}

string Controller::getCurrentTime() const {
    time_t now = time(0);
    return ctime(&now);
}

int Controller::getFirstClusterAvailable() const {
    for (size_t i = 0; i < clusters.size(); ++i)
        if (clusters[i].data.empty())
            return i;
    return clusters.size();
}

bool Controller::existFile(const string &path, const string &name) const {
    return tables.count(path) && tables.at(path).count(name);
}

bool Controller::existDirectory(const string &path) const {
    return tables.count(path);
}

File Controller::createFile(const string &path, const string &name, const string &data, const string &permissions) {
    Metadata metadata(getCurrentTime(), getFileExtension(name), true, path, permissions, data.length());
    return File(metadata, data);
}

File Controller::createDirectory(const string &path, const string &permissions) {
    Metadata metadata(getCurrentTime(), "", false, path, permissions, 0);
    return File(metadata, "");
}

string Controller::saveFile(const string &path, const string &name, const string &data, const string &permissions) {
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
        if (clusterTemp == static_cast<int>(clusters.size()))
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

string Controller::saveDirectory(const string &path, const string &name, const string &permissions) {
    if (!endsWith(path, "/")) return "Path no válido";
    if (name.find("/") != string::npos) return "Nombre no válido";
    if (!existDirectory(path)) return "Ruta no existente";
    if (existDirectory(path + name + "/")) return "Carpeta ya existente";

    tables[path + name + "/"]["."] = DirecTable(".", -1, Metadata());
    tables[path + name + "/"][".."] = DirecTable("..", -1, Metadata());
    tables[path][name] = DirecTable(name, -1, Metadata(getCurrentTime(), "", false, path + name + "/", permissions, 0));
    return "CREATED";
}

string Controller::deleteFile(const string &path, const string &name) {
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

string Controller::viewClusters() const {
    string res;
    for (const auto &cluster : clusters)
        res += cluster.toString() + "\n";
    return res;
}

string Controller::copyFile(const string &sourcePath, const string &fileName, const string &destPath) {
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

    while (clusterNumber != -1) {
        fileData += clusters[clusterNumber].data;
        clusterNumber = clusters[clusterNumber].next;
    }

    return saveFile(destPath, fileName, fileData, sourceTable.metadata.permissions);
}

string Controller::moveFile(const string &sourcePath, const string &fileName, const string &destPath) {
    string copyResult = copyFile(sourcePath, fileName, destPath);
    if (copyResult == "CREATED") {
        return deleteFile(sourcePath, fileName);
    }
    return copyResult;
}

void Controller::printTables() {
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

void Controller::init() {
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
    cout << moveFile("/", "archivob.txt", "/root/") << endl;
    printTables();
    cout << viewClusters() << endl;
}
