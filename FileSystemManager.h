#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>
#include <utility>

// Clase para comunicarse con el DeviceManager
#include "IORequest.h"

using namespace std;

// =====================
// Clase Metadata
// =====================
class Metadata {
public:
    string createDate;
    string extension;
    bool file;
    string path;
    string permissions;
    int tam;

    Metadata();
    Metadata(string createDate, string extension, bool file, string path, string permissions, int tam);
};

// =====================
// Clase Cluster
// =====================
class Cluster {
public:
    int number;
    string data;
    int next;

    Cluster();
    Cluster(int number, const string &data, int next);
    string toString() const;
};

// =====================
// Clase DirecTable
// =====================
class DirecTable {
public:
    string name;
    int cluster;
    Metadata metadata;

    DirecTable();
    DirecTable(string name, int cluster, Metadata metadata);
    string toString() const;
};

// =====================
// Clase File
// =====================
class File {
public:
    Metadata metadata;
    string data;

    File();
    File(Metadata metadata, const string &data);
};

// =====================
// Clase Controller
// =====================
class Controller {
private:
    map<string, map<string, DirecTable>> tables;
    vector<Cluster> clusters;
    int clusterTam;

    bool endsWith(const string &str, const string &suffix) const;
    string getFileExtension(const string &name) const;
    string getCurrentTime() const;
    int getFirstClusterAvailable() const;
    bool existFile(const string &path, const string &name) const;
    bool existDirectory(const string &path) const;

public:
    Controller();

    File createFile(const string &path, const string &name, const string &data, const string &permissions);
    File createDirectory(const string &path, const string &permissions);

    string saveFile(const string &path, const string &name, const string &data, const string &permissions);
    string saveDirectory(const string &path, const string &name, const string &permissions);
    string deleteFile(const string &path, const string &name);
    string viewClusters() const;
    string copyFile(const string &sourcePath, const string &fileName, const string &destPath);
    string moveFile(const string &sourcePath, const string &fileName, const string &destPath);

    void printTables();
    void init();
};

#endif // FILESYSTEM_H
