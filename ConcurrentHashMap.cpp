using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <typeinfo>

unsigned int ConcurrentHashMap::hash(string s) {
    return (int) (s.at(0)) - 97;
}

void ConcurrentHashMap::processFile(string arch) {
    ifstream archivo;
    string linea;
    archivo.open(arch.c_str());
    while (archivo >> linea) {
        //cerr << linea << endl;
        this->addAndInc(linea);
    }
    archivo.close();
}

void* ConcurrentHashMap::f(void* cosa) {
    Cosa* c = (Cosa*) cosa;
    //    cerr << "llame a cosa con " << c->_arch << " <-" << endl;
    c->_hashMap->processFile(c->_arch);
    if (c->_conMutex) {
        pthread_mutex_t &m = c->_mutex;
        free(cosa);
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

ConcurrentHashMap::ConcurrentHashMap() {
    for (int i = 0; i < 26; i++) {
        this->tabla[i] = new Lista<item>();
        pthread_mutex_init(&aai[i], NULL);
    }
}

ConcurrentHashMap::~ConcurrentHashMap() {
    //    for (int i = 0; i < 26; i++) {
    //        free(this->tabla[i]);
    //        pthread_mutex_destroy(&aai[i]);
    //    }
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
    ConcurrentHashMap* hashMap = new ConcurrentHashMap();
    unsigned int cant_hilos = archs.size();
    pthread_t hilos[cant_hilos];
    Cosa * cosa[cant_hilos];
    int h_id = 0;
    for (list<string>::iterator it = archs.begin(); it != archs.end(); ++it) {
        //SIN HILOS
        //hashMap->processFile(*it);
        //CON HILOS
        cosa[h_id] = new Cosa(hashMap, *it);
        pthread_create(&(hilos[h_id]), NULL, f, (void*) (cosa[h_id]));
        h_id++;
    }
    for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilos[h_id], NULL);
        free(cosa[h_id]);
    }
    return *hashMap;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    ConcurrentHashMap* hashMap = new ConcurrentHashMap();
    //    unsigned int cant_archivos = archs.size();
    //    unsigned int cant_hilos = min(n, cant_archivos); //voy a crear a lo sumo cant achivos hilos por mas que n sea mayor
    //    pthread_t hilos[cant_hilos];
    //    pthread_mutex_t mutex[cant_hilos];
    //    int h_id = 0;
    //    Cosa * cosa[cant_hilos];
    //    list<string>::iterator it = archs.begin();
    //
    //    for (h_id = 0; h_id < cant_hilos; h_id++) {//pongo a correr en todos los hilos un archivo
    //        pthread_mutex_init(&mutex[cant_hilos], NULL);
    //        cosa[h_id] = new Cosa(hashMap, *it, h_id, mutex[h_id]);
    //        pthread_create(&(hilos[h_id]), NULL, f, (void*) (cosa[h_id]));
    //        ++it;
    //    }//fin primera asignacion
    //    h_id = 0;
    //    while (it != archs.end()) {//ciclo para el resto de archivos con todos los hilos llenos
    //        while (pthread_mutex_trylock(&mutex[h_id])) {//espero a que alguno se vacie (una especie de busy waiting?)
    //            h_id = (h_id++ % n);
    //        }
    //        cosa[h_id] = new Cosa(hashMap, *it, h_id, mutex[h_id]);
    //        pthread_create(&(hilos[h_id]), NULL, f, (void*) (cosa[h_id]));
    //        ++it;
    //        h_id = (h_id++ % n);
    //    }
    return *hashMap;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    return make_pair("Hola", 3);
}
