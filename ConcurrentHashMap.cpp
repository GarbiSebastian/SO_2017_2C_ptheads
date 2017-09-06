using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <semaphore.h>

pthread_mutex_t tomoArchivo;

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
    return NULL;
}

ConcurrentHashMap::ConcurrentHashMap() {
    for (int i = 0; i < maxLength; i++) {
        this->tabla[i] = new Lista<item>();
        pthread_mutex_init(&aai[i], NULL);
    }
}

ConcurrentHashMap::~ConcurrentHashMap() {
    for (int i = 0; i < maxLength; i++) {
        //free(this->tabla[i]);
        pthread_mutex_destroy(&aai[i]);
    }
}

void ConcurrentHashMap::addAndInc(string key) {
    item* t;
    bool encontre = false;
    int hashKey = this->hash(key);
    assert(hashKey < maxLength);
    pthread_mutex_lock(&(aai[hashKey])); //pido mutex correspondiente a la letra para insertar
    auto it = this->tabla[hashKey]->CrearIt();
    while (it.HaySiguiente() && !encontre) {
        t = &(it.Siguiente());
        encontre = t->first == key;
        it.Avanzar();
    }

    if (encontre) {
        t->second++;
    } else {
        this->tabla[hashKey]->push_front(make_pair(key, 1));
    }
    pthread_mutex_unlock(&(aai[hashKey]));
}

bool ConcurrentHashMap::member(string key) {

    int hashKey = this->hash(key);
    auto it = this->tabla[hashKey]->CrearIt();
    while (it.HaySiguiente()) {

        if (it.Siguiente().first == key) {
            return true;
        } else {
            it.Avanzar();
        }

    }
    return false;
}

void* procesarFila(void* lista) {

    string maxKey = "";
    int maxValue = 0;
    auto it = ((Lista<item>*) lista)->CrearIt();

    while (it.HaySiguiente()) {
        item itemActual = it.Siguiente();

        if (maxValue < itemActual.second) {
            maxKey = itemActual.first;
            maxValue = itemActual.second;
        }

        it.Avanzar();
    }

    return (void*) (new item(maxKey, maxValue));
    //return (void*) (make_pair(maxKey, maxValue));
}

item ConcurrentHashMap::maximum(unsigned int nt) {

    for (int i = 0; i < maxLength; i++) {
        pthread_mutex_lock(&(aai[i])); // Bloqueo todo el  array
    }

    string maxKey = "";
    int maxValue = 0;
    item * maximosXFila[maxLength];
    int filaActual = 0;

    pthread_t threads[nt];

    // concurrencia interna
    // se completa con los max locales de c/fila el aray maximosXFila
    while (filaActual < maxLength) {

        // Se lanzan los nt threads (o los que restan por lanzar)
        for (int i = 0; i < min(nt, maxLength - filaActual); i++) {
            pthread_create(&(threads[i]), NULL, &procesarFila, tabla[filaActual + i]);
        }

        // Recolecto el resultado de la ejecucion de los threads,
        // y si no terminaron se los espera
        for (int i = 0; i < min(nt, maxLength - filaActual); i++) {
            pthread_join(threads[i], (void**) (&maximosXFila[filaActual + i]));
        }

        filaActual += nt;
    }

    // se busca el maximo del ConcurentHasmap
    for (int i = 0; i < maxLength; i++) {
        if (maxValue < maximosXFila[i]->second) {
            maxKey = maximosXFila[i]->first;
            maxValue = maximosXFila[i]->second;
        }

        delete maximosXFila[i];
    }

    for (int i = 0; i < maxLength; i++) {
        pthread_mutex_unlock(&(aai[i])); // Desbloqueo todo el  array
    }

    return make_pair(maxKey, maxValue);
}

ConcurrentHashMap ConcurrentHashMap::count_words(string arch) {
    ConcurrentHashMap hashMap;
    hashMap.processFile(arch);
    return hashMap;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs) {
    ConcurrentHashMap* hashMap = new ConcurrentHashMap();
    unsigned int cant_hilos = archs.size();
    pthread_t hilo[cant_hilos];
    Cosa * cosa[cant_hilos];
    int h_id = 0;
    for (list<string>::iterator it = archs.begin(); it != archs.end(); ++it) {
        //SIN HILOS
        //hashMap->processFile(*it);
        //CON HILOS
        cosa[h_id] = new Cosa(hashMap, *it);
        pthread_create(&(hilo[h_id]), NULL, f, (void*) (cosa[h_id]));
        h_id++;
    }
    for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilo[h_id], NULL);
        free(cosa[h_id]);
    }
    return *hashMap;
}

void* ConcurrentHashMap::g(void* c){
	Cosa* cosa = (Cosa*) c;
	ConcurrentHashMap* hashMap= cosa->_hashMap;
	string arch;
	pthread_mutex_lock(&tomoArchivo);
	while(*(cosa->_it) != *(cosa->_end)){
		arch = *(*(cosa->_it));
		++(*(cosa->_it));
		pthread_mutex_unlock(&tomoArchivo);
		hashMap->processFile(arch);
		pthread_mutex_lock(&tomoArchivo);
	}
	pthread_mutex_unlock(&tomoArchivo);
    return NULL;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
	ConcurrentHashMap* hashMap = new ConcurrentHashMap();
	unsigned int cant_archivos = archs.size();
    unsigned int cant_hilos = min(n, cant_archivos);
    Cosa* cosa[cant_hilos];
    pthread_t hilo[cant_hilos];
	list<string>::iterator it = archs.begin();
	list<string>::iterator end = archs.end();
	pthread_mutex_init(&tomoArchivo,NULL);
	unsigned int h_id = 0;
	for (h_id = 0; h_id < cant_hilos; h_id++) {//pongo a correr en todos los hilos un archivo
		cosa[h_id] = new Cosa(hashMap, &it, &end);
        pthread_create(&(hilo[h_id]), NULL, g, (void*) (cosa[h_id]));
    }
	for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilo[h_id], NULL);
    }	
	return *hashMap;
}

void* ConcurrentHashMap::crear_hashMaps(void* c){
	Cosa2* cosa2 = (Cosa2*) c;
	string arch;
	pthread_mutex_lock(&tomoArchivo);
	while(*(cosa2->_it) != *(cosa2->_end)){
		arch = *(*(cosa2->_it));
		++(*(cosa2->_it));
		pthread_mutex_unlock(&tomoArchivo);
		ConcurrentHashMap hashMap = ConcurrentHashMap::count_words(arch);
        	cosa2->_hashMaps->push_front(hashMap);
		pthread_mutex_lock(&tomoArchivo);
	}
	pthread_mutex_unlock(&tomoArchivo);
    	return NULL;
}

void ConcurrentHashMap::add_hashMaps(item* p) {
    item* t;
    bool encontre = false;
    int hashKey = this->hash(p->first);
    //assert(hashKey < maxLength);
    pthread_mutex_lock(&(aai[hashKey])); //pido mutex correspondiente a la letra para insertar
    auto it = this->tabla[hashKey]->CrearIt();
    while (it.HaySiguiente() && !encontre) {
        t = &(it.Siguiente());
        encontre = t->first == p->first;
        it.Avanzar();
    }
    if (encontre) {
        t->second = t->second + p->second;
    } else {
        this->tabla[hashKey]->push_front(*p);
    }
    pthread_mutex_unlock(&(aai[hashKey]));
}

item ConcurrentHashMap::maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    Lista<ConcurrentHashMap>* hashMaps;
    ConcurrentHashMap* hashMap_principal = new ConcurrentHashMap();
    unsigned int cant_archivos = archs.size();
    unsigned int cant_hilos = min(p_archivos, cant_archivos);
    Cosa2* cosa2[cant_hilos];
    pthread_t hilo[cant_hilos];
	list<string>::iterator it = archs.begin();
	list<string>::iterator end = archs.end();
	pthread_mutex_init(&tomoArchivo,NULL);
    unsigned int h_id = 0;
    
    //armo la lista de hashMaps
	for (h_id = 0; h_id < cant_hilos; h_id++) {
		cosa2[h_id] = new Cosa2(hashMaps, &it, &end);
        pthread_create(&(hilo[h_id]), NULL, crear_hashMaps, (void*) (cosa2[h_id]));
    }
	for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilo[h_id], NULL);
    }
    
    //merge (poner todos los "hashMaps" en hashMap_principal)
    ConcurrentHashMap* t = new ConcurrentHashMap();
    item* t2;
    it = archs.begin();
    end = archs.end();
    h_id = 0;
    unsigned int i = 0;
    auto it1 = hashMaps->CrearIt();
    while (it1.HaySiguiente()) {
        t = &(it1.Siguiente());
        for (i = 0; i < maxLength; i++) {
            auto it2 = t->tabla[i]->CrearIt();
            while (it2.HaySiguiente()) {
                t2 = &(it2.Siguiente());
                hashMap_principal->add_hashMaps(t2);
                it2.Avanzar();
            }
        }
        it1.Avanzar();
    }
    
    return hashMap_principal->maximum(p_maximos);
}

/*
item ConcurrentHashMap::maximum(unsigned int p_maximos, list<string> archs) {
    ConcurrentHashMap hashMap;
    for (list<string>::iterator it = archs.begin(); it != archs.end(); ++it) {
        hashMap.processFile(*it);
    }
    return hashMap.maximum(p_maximos);
}
*/

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    ConcurrentHashMap hashMap = ConcurrentHashMap::count_words(p_archivos, archs);
    return hashMap.maximum(p_maximos);
}
