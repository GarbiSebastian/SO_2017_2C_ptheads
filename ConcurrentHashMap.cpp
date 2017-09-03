using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <typeinfo>
#include <algorithm>

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
    //    for (int i = 0; i < maxLength; i++) {
    //        free(this->tabla[i]);
    //        pthread_mutex_destroy(&aai[i]);
    //    }
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
        t->second++; //esto no estaria funcionando
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
        }else{
            it.Avanzar();    
        }
        
    }
    return false;
}


void* procesarFila(void * l) {

    Lista<item>* lista = (Lista<item>*) l;

    item* maximo = NULL;
    item* itemActual;

    auto it = lista->CrearIt();
    while (it.HaySiguiente()) { 
        itemActual = &(it.Siguiente());
        if (maximo != NULL && maximo->second > itemActual->second) {
            maximo = itemActual;
        }

        it.Avanzar();
    }

    cout << "max: " << maximo << endl; 
    return maximo;
    //pthread_exit(maximo);
}

item ConcurrentHashMap::maximum(unsigned int nt) {

    for (int i= 0; i < maxLength; i++) {
       pthread_mutex_lock(&(aai[i])); // Bloqueo todo el  array
    }

    item* maximo = new item("TEST-SO", 0) ;//NULL;
    item* maximosXFila[maxLength];
    int filaActual = 0;

    pthread_t threads[nt];

    // concurrencia interna 
    // se completa con los max locales de c/fila el aray maximosXFila 
    while(filaActual < maxLength) {

        // Se lanzan los nt threads (o los que restan por lanzar) 
        for (int i = 0; i < min(nt, maxLength - filaActual); i++) {
            pthread_create(&(threads[i]), NULL, &procesarFila, &(tabla[filaActual + i]));
        }

        // Recolecto el resultado de la ejecucion de los threads,
        // y si no terminaron se los espera 
        for (int i = 0; i < min(nt, maxLength - filaActual); i++){
            pthread_join(threads[i], (void**) (&(maximosXFila[filaActual + i])));         
        } 

        filaActual += nt;
    }

    // se busca el maximo del ConcurentHasmap
    for(int i = 0; i < maxLength; i++) {
        /*if(maximo->second < maximosXFila[i]->second) {
            maximo = maximosXFila[i];
        }*/

        cout << maximosXFila[i] << endl;
    }

    for (int i = 0; i < maxLength; i++) {
        pthread_mutex_unlock(&(aai[i])); // Desbloqueo todo el  array
    }

    return *maximo;
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
    //    pthread_t hilos[n];
    //    Cosa * cosa[n];
    //    int h_id = 0;
    //    for (list<string>::iterator it = archs.begin(); it != archs.end(); ++it) {
    //        //SIN HILOS
    //        //hashMap->processFile(*it);
    //        //CON HILOS
    //        cosa[h_id] = new Cosa(hashMap, *it);
    //        pthread_create(&(hilos[h_id]), NULL, f, (void*) (cosa[h_id]));
    //        h_id++;
    //    }
    //    for (h_id = 0; h_id < cant_archivos; h_id++) {
    //        pthread_join(hilos[h_id], NULL);
    //        free(cosa[h_id]);
    //    }
    return *hashMap;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    return make_pair("Hola", 3);
}
