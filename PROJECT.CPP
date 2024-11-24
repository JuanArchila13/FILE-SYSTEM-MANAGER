#include <iostream>
#include <vector>
#include <map>
#include <cstring>
#include <ctime>

using namespace std;

struct Metadata{
    int tam;
    bool file;
    string path;
    string extension;
    string permissions;
    time_t createDate;
}; 

struct DirecTable{
    string name;
    int cluster;
    Metadata metadata;
};

struct Cluster {
    int number;
    string data;
    int next;
    Cluster() : number(-1), data(""), next(-1) {}
};

map<string, DirecTable> tables;
vector<Cluster> clusters;
int clusterTam = 32;

void createFile(string path, string name, string data){
    int tam = data.length();
    int tempTam = tam;

    while (tempTam > clusterTam){

    }
    cout<<name<<" "<<data<<" "<<tam;
}

int firstClusterEmpty(){
	for(int i = 0; i < vector.size(); i++){
		if (clusters[i].number == -1) {
			return i;
		}
	}
	return -1;
}

int main(){
    createFile("/", "File1.txt", "{datos}");
    cout<<"Hola";
    return 0;
}
