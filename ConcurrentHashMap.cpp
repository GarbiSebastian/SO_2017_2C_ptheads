using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <semaphore.h>

pthread_mutex_t tomoArchivo;

ConcurrentHashMap::ConcurrentHashMap() {
    for (int i = 0; i < maxLength; i++) {
        this->tabla[i] = new Lista<item>();
        //        pthread_mutex_init(&aai[i], NULL);
        pthread_rwlock_init(&aai[i], NULL);
    }
}

ConcurrentHashMap::~ConcurrentHashMap() {
    for (int i = 0; i < maxLength; i++) {
        //free(this->tabla[i]);
        //        pthread_mutex_destroy(&aai[i]);
        pthread_rwlock_destroy(&aai[i]);
    }
}

unsigned int ConcurrentHashMap::hash(string s) {
    return (int) (s.at(0)) - 97;
}

void ConcurrentHashMap::addAndInc(string key) {
    return this->addAndInc(key,1);
}

void ConcurrentHashMap::addAndInc(string key, unsigned int n) {
    item* t;
    bool encontre = false;
    int hashKey = this->hash(key);
    assert(hashKey < maxLength);
    auto it = this->tabla[hashKey]->CrearIt();

    //CASO LISTA VACÍA (PARA QUE TERMINE RÁPIDO EN CASO DE TENER LA LISTA VACÍA)
    if (!it.HaySiguiente()) {
        //CASO LISTA VACÍA
        pthread_rwlock_wrlock(&(aai[hashKey])); //pido rw_lock correspondiente a la letra para insertar
        it = this->tabla[hashKey]->CrearIt();    
        if (!it.HaySiguiente()) {// La lista sigue vacía (Nadie más insertó desde que pregunté) pero esta vez solo yo puedo escribir
            this->tabla[hashKey]->push_front(make_pair(key, n));
            pthread_rwlock_unlock(&(aai[hashKey]));
            return;
        }else{
            //SI ALGUIEN AGREGÓ ALGO DESPUES DEL PRIMER IF, LA LISTA YA NO ESTÁ VACÍA, DEVUELVO EL LOCK Y ENTRA POR EL CASO LISTA CON ELEMENTOS
            pthread_rwlock_unlock(&(aai[hashKey]));
        }
    }
    
    //CASO LISTA CON ELEMENTOS
    t = &(it.Siguiente());
    string primeraKey = t->first;//guardo el primer elemento para después
    encontre = t->first == key;
    it.Avanzar();
    
    while (it.HaySiguiente() && !encontre) {//HAGO UNA PRIMERA BÚSQUEDA SIN BLOQUEAR
        t = &(it.Siguiente());
        encontre = t->first == key;
        it.Avanzar();
    }

    
    if (encontre) {
        //CASO EXISTÍA LA CLAVE
        pthread_rwlock_wrlock(&(aai[hashKey])); //pido rw_lock correspondiente a la letra para insertar
        t->second+=n;
        pthread_rwlock_unlock(&(aai[hashKey]));
    } else {
        //REVISÉ TODA LA LISTA Y NO APARECIÓ. TENGO QUE VER SI NADIE LA AGREGÓ DESDE QUE EMPECÉ A CHEQUEAR
        pthread_rwlock_wrlock(&(aai[hashKey])); //pido rw_lock correspondiente a la letra para insertar
        it = this->tabla[hashKey]->CrearIt();
        t = &(it.Siguiente());
        encontre = t->first == key;
        it.Avanzar();
        while (it.HaySiguiente() && !encontre && primeraKey != t->first) {//REVISO DE NUEVO LA PRIMERA PARTE DE LA LISTA, ESTA VEZ CON LOCK DE ESCRITURA TOMADO
            t = &(it.Siguiente());
            encontre = t->first == key;    
            it.Avanzar();
        }
        if (encontre) {
            //CASO EN QUE OTRO PROCESO LA INSERTÓ ANTES QUE YO EN LA PRIMERA BÚSQUEDA
            t->second+=n;
        } else {
            this->tabla[hashKey]->push_front(make_pair(key, n));
        }
        pthread_rwlock_unlock(&(aai[hashKey]));
    }
    
    return;
}

void ConcurrentHashMap::procesarArchivo(string arch) {
    ifstream archivo;
    string palabra;
    archivo.open(arch.c_str());
    while (archivo >> palabra) {
        //cerr << linea << endl;
        this->addAndInc(palabra);
    }
    archivo.close();
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

void* ConcurrentHashMap::calcularMaximos(void* parametro){
    item* itemMaximo = new item("",0);
    item* itemActual;
    unsigned int hashKey;
    ParametroMaximum* p = (ParametroMaximum*) parametro;
    hashKey = p->_proximaLetra++;
    while(hashKey < maxLength){
        auto it = ((Lista<item>*) p->_hashMap->tabla[hashKey])->CrearIt();
        while (it.HaySiguiente()) {
            itemActual = &(it.Siguiente());
            if (itemMaximo->second < itemActual->second) {
                itemMaximo = itemActual;
            }
            it.Avanzar();
        }
        hashKey = p->_proximaLetra++;    
    }
    return (void*) itemMaximo;
}

item ConcurrentHashMap::maximum(unsigned int nt) {
    unsigned int m = maxLength;
    unsigned int cant_hilos = min(nt,m);
    
    pthread_t threads[cant_hilos];
    ParametroMaximum parametro(this);
    //tomo todos los rw_locks para evitar la escritura en la tabla
    for(int i = 0; i< maxLength; i++){
        pthread_rwlock_rdlock(&(this->aai[i]));
    }
    for (int i = 0; i < cant_hilos; i++){
        pthread_create(&(threads[i]), NULL, &calcularMaximos, &parametro);
    }
    item* siguiente;
    item* maximo;
    pthread_join(threads[0], (void**) &maximo);
    //cerr << "maximo= " << maximo->first << " : " << maximo->second << endl;    
    for (int i = 1; i < cant_hilos; i++) {
        pthread_join(threads[i], (void**) &siguiente);
        if(maximo->second < siguiente->second){
            maximo = siguiente;
        }
    }
    //devuelvo todos los rw_locks
    for(int i = 0; i< maxLength; i++){
        pthread_rwlock_unlock(&(this->aai[i]));
    }
    return *maximo;
}

ConcurrentHashMap ConcurrentHashMap::count_words(string arch) {
    ConcurrentHashMap hashMap;
    hashMap.procesarArchivo(arch);
    return hashMap;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs) {
    ConcurrentHashMap* hashMap = new ConcurrentHashMap();
    unsigned int cant_hilos = archs.size();
    pthread_t hilo[cant_hilos];
    ParametroProcesarArchivo * parametro[cant_hilos];
    int h_id = 0;
    for (list<string>::iterator it = archs.begin(); it != archs.end(); ++it) {
        //SIN HILOS
        //hashMap->processFile(*it);
        //CON HILOS
        parametro[h_id] = new ParametroProcesarArchivo(hashMap, *it);
        pthread_create(&(hilo[h_id]), NULL, hiloProcesarArchivo, (void*) (parametro[h_id]));
        h_id++;
    }
    for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilo[h_id], NULL);
        free(parametro[h_id]);
    }
    return *hashMap;
}

void* ConcurrentHashMap::hiloProcesarArchivo(void* parametro) {
    ParametroProcesarArchivo* p = (ParametroProcesarArchivo*) parametro;
    p->_hashMap->procesarArchivo(p->_arch);
    return NULL;
}

void* ConcurrentHashMap::procesarListaDeArchivos(void* parametro) {
    ParametroProcesarListaDeArchivos* p = (ParametroProcesarListaDeArchivos*) parametro;
    string arch;
    pthread_mutex_lock(&tomoArchivo);
    while (*(p->_it) != *(p->_end)) {
        arch = *(*(p->_it));
        ++(*(p->_it));
        pthread_mutex_unlock(&tomoArchivo);
        p->_hashMap->procesarArchivo(arch);
        pthread_mutex_lock(&tomoArchivo);
    }
    pthread_mutex_unlock(&tomoArchivo);
    return NULL;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    ConcurrentHashMap* hashMap = new ConcurrentHashMap();
    unsigned int cant_archivos = archs.size();
    unsigned int cant_hilos = min(n, cant_archivos);
    pthread_t hilo[cant_hilos];
    list<string>::iterator it = archs.begin();
    list<string>::iterator end = archs.end();
    ParametroProcesarListaDeArchivos* parametro = new ParametroProcesarListaDeArchivos(hashMap, &it, &end);
    pthread_mutex_init(&tomoArchivo, NULL);
    for (unsigned int i = 0; i < cant_hilos; i++) {
        pthread_create(&(hilo[i]), NULL, procesarListaDeArchivos, (void*) parametro);
    }
    for (unsigned int i = 0; i < cant_hilos; i++) {
        pthread_join(hilo[i], NULL);
    }
    return *hashMap;
}

void* ConcurrentHashMap::crear_hashMaps(void* parametro) {
    ParametroCrearHashMaps* p = (ParametroCrearHashMaps*) parametro;
    string arch;
    pthread_mutex_lock(&tomoArchivo);
    while (*(p->_it) != *(p->_end)) {
        arch = *(*(p->_it));
        ++(*(p->_it));
        pthread_mutex_unlock(&tomoArchivo);
        ConcurrentHashMap hashMap = ConcurrentHashMap::count_words(arch);
        p->_hashMaps->push_front(hashMap);
        pthread_mutex_lock(&tomoArchivo);
    }
    pthread_mutex_unlock(&tomoArchivo);
    return NULL;
}

item ConcurrentHashMap::maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    Lista<ConcurrentHashMap>* hashMaps = new Lista<ConcurrentHashMap>();
    ConcurrentHashMap* hashMap_principal = new ConcurrentHashMap();
    unsigned int cant_archivos = archs.size();
    unsigned int cant_hilos = min(p_archivos, cant_archivos);
    pthread_t hilo[cant_hilos];
    list<string>::iterator it = archs.begin();
    list<string>::iterator end = archs.end();
    ParametroCrearHashMaps * parametro = new ParametroCrearHashMaps(hashMaps, &it, &end);
    
    pthread_mutex_init(&tomoArchivo, NULL);
    unsigned int h_id = 0;
    //armo la lista de hashMaps
    for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_create(&(hilo[h_id]), NULL, crear_hashMaps, (void*) parametro);
    }
    for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilo[h_id], NULL);
    }
    
    //merge (poner todos los "hashMaps" en hashMap_principal)
    ConcurrentHashMap* hashMap_p;
    item* item_p;
    auto it_hashMaps = hashMaps->CrearIt();
    while (it_hashMaps.HaySiguiente()) {
        hashMap_p = &(it_hashMaps.Siguiente());
        for (unsigned int i = 0; i < maxLength; i++) {
            auto it_listaItem = hashMap_p->tabla[i]->CrearIt();
            while (it_listaItem.HaySiguiente()) {
                item_p = &(it_listaItem.Siguiente());
                hashMap_principal->addAndInc(item_p->first,item_p->second);
                it_listaItem.Avanzar();
            }
        }
        it_hashMaps.Avanzar();
    }

    return hashMap_principal->maximum(p_maximos);
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    ConcurrentHashMap hashMap = ConcurrentHashMap::count_words(p_archivos, archs);
    return hashMap.maximum(p_maximos);
}
