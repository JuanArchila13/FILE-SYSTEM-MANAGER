#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <cmath>
#include <sstream>
#include "Cluster.h"
#include "DirecTable.h"
#include "File.h"
#include "Metadata.h"

using namespace std;

class FileSystemManager {
private:
    map<string, map<string, DirecTable>> mainTables;
    vector<Cluster> mainClusters;
    vector<Cluster> swapClusters;
    int clusterTam;
    int diskSize;

public:
    FileSystemManager() {
        mainTables["/"] = {};
        mainTables["/"]["."] = DirecTable(".", -1, Metadata());
        diskSize = obtainDiskSize();
        mainClusters.resize(diskSize);
        swapClusters.resize(100);
        clusterTam = 32;
    }

    File createFile(const string &path, const string &name, const string &data, const string &permissions) {
        Metadata metadata(
            ctime(nullptr),
            name.substr(name.find_last_of('.')),
            true,
            path,
            permissions,
            data.size()
        );
        return File(metadata, data);
    }

    int obtainDiskSize() {
        return 400;
    }

    bool execFile(const string &filePath, const string &fileName) {
        if (getTable(filePath).count(fileName)) {
            // Aquí se llamaría al sistema de procesos para crear el PID
            return true;
        }
        return false;
    }

    File createDirectory(const string &path, const string &permissions) {
        Metadata metadata(
            ctime(nullptr),
            "",
            false,
            path,
            permissions,
            0
        );
        return File(metadata, "");
    }

    string saveFile(const string &path, const string &name, const string &data, const string &permissions) {
        if (path.back() != '/')
            return "path no valido";
        if (!existDirectory(path))
            return "path no existente";
        if (existFile(path, name))
            return "archivo ya existente";
        if (numClustersRequired(data) > availableNumberOfClusters())
            return "sin espacio disponible";

        File file = createFile(path, name, data, permissions);
        string remainingData = file.data;
        int clusterTemp = -1;
        int previousClusterTemp = -1;
        int remaining = remainingData.size();
        DirecTable direcTable(name, -1, file.metadata);

        while (!remainingData.empty()) {
            int tamSave = (remaining > clusterTam) ? clusterTam : remaining;
            string saverData = remainingData.substr(0, tamSave);
            remainingData = remainingData.substr(tamSave);
            remaining = remainingData.size();
            clusterTemp = getFirstClusterAvailable();
            Cluster cluster(clusterTemp, saverData, -1);
            mainClusters[clusterTemp] = cluster;

            if (previousClusterTemp != -1)
                mainClusters[previousClusterTemp].next = clusterTemp;
            else {
                direcTable.cluster = clusterTemp;
                getTable(path)[direcTable.name] = direcTable;
            }
            previousClusterTemp = clusterTemp;
        }
        return "CREATED";
    }

    string saveDirectory(const string &path, const string &name, const string &permissions) {
        if (path.back() != '/')
            return "path no valido";
        if (name.find('/') != string::npos)
            return "nombre no valido, no ingrese /";
        if (!existDirectory(path))
            return "ruta no existente";
        if (existDirectory(path + name + "/"))
            return "carpeta ya existente";

        mainTables[path + name + "/"] = {};
        getTable(path)[name] = DirecTable(name, -1, Metadata(ctime(nullptr), "", false, path, permissions, 0));
        getTable(path + name + "/")["."] = DirecTable(".", -1, Metadata());
        getTable(path + name + "/")[".."] = DirecTable("..", -1, Metadata());

        return "CREATED " + path + name + "/";
    }

    string deleteFile(const string &pathAndName) {
        string path = pathAndName.substr(0, pathAndName.find_last_of('/') + 1);
        string name = pathAndName.substr(pathAndName.find_last_of('/') + 1);
        return deleteFile(path, name);
    }

    string deleteFile(const string &path, const string &name) {
        if (!existDirectory(path))
            return "ruta no existente";
        if (!existFile(path, name))
            return "archivo no existente";

        auto &table = getTable(path);
        DirecTable direcTable = table[name];
        int clusterNumber = direcTable.cluster;
        while (clusterNumber != -1) {
            Cluster cluster = mainClusters[clusterNumber];
            mainClusters[clusterNumber] = Cluster();
            clusterNumber = cluster.next;
        }
        table.erase(name);
        return "DELETED";
    }

    string listDirectory(const string &path) {
        const auto &table = getTable(path.back() != '/' ? path + "/" : path);
        string res;
        for (const auto &entry : table) {
            const DirecTable &direcTable = entry.second;
            res += direcTable.name + " -- " + (direcTable.metadata.file ? "file" : "directory") + "\n";
        }
        return res;
    }

private:
    map<string, DirecTable> &getTable(const string &path) {
        return mainTables[path];
    }

    bool existFile(const string &path, const string &name) {
        return getTable(path).count(name);
    }

    bool existDirectory(const string &path) {
        return mainTables.count(path);
    }

    int getFirstClusterAvailable() {
        for (size_t i = 0; i < mainClusters.size(); ++i)
            if (mainClusters[i].number == -1)
                return i;
        return mainClusters.size();
    }

    int numClustersRequired(const string &data) {
        return ceil(static_cast<double>(data.size()) / clusterTam);
    }

    int availableNumberOfClusters() {
        int num = 0;
        for (int i = 0; i < mainClusters.size(); i++) {
            if (mainClusters[i].number == -1) // Verifica si el cluster está "vacío"
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

    int main() {
    FileSystemManager controller;
    controller.init();
    return 0;
}

};
