using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

unsigned int ConcurrentHashMap::hash(string s) {
    return (int) (s.at(0)) - 97;
}

void ConcurrentHashMap::processFile(string arch) {
    ifstream archivo;
    string linea;
    archivo.open(arch.c_str());
    while (!archivo.eof()) {
        archivo >> linea;
        //cout << linea << endl;
        this->addAndInc(linea);
    }
    archivo.close();
}

ConcurrentHashMap::ConcurrentHashMap() {
    for (int i = 0; i < 26; i++) {
        this->tabla[i] = new Lista<item>();
        pthread_mutex_init(&aai[i], NULL);
    }
}

void ConcurrentHashMap::addAndInc(string key) {
    item* t;
    bool encontre = false;
    int hashKey = this->hash(key);
    assert(hashKey < 26);
    pthread_mutex_lock(&(aai[hashKey])); //pido mutex correspondiente a la letra para insertar
    auto it = this->tabla[hashKey]->CrearIt();
    while (it.HaySiguiente() && !encontre) {
        t = &(it.Siguiente());
        encontre = t->first == key;
        it.Avanzar();
    }

    if (encontre) {
        t->second++; //esto no estaria funcionando
    } else {
        this->tabla[hashKey]->push_front(make_pair(key, 1));
    }
    pthread_mutex_unlock(&(aai[hashKey]));
}

bool ConcurrentHashMap::member(string key) {
    return true;
}

item ConcurrentHashMap::maximum(unsigned int nt) {
    return make_pair("Hola", 3);
}

ConcurrentHashMap ConcurrentHashMap::count_words(string arch) {
    ConcurrentHashMap hashMap;
    hashMap.processFile(arch);
    return hashMap;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs) {
    ConcurrentHashMap a;
    return a;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    ConcurrentHashMap a;
    return a;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    return make_pair("Hola", 3);
}
