using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <semaphore.h>

sem_t sem_c;
pthread_mutex_t libre_mutex;
bool *libre;

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

void* ConcurrentHashMap::g(void* cosa) {
    Cosa* c = (Cosa*) cosa;
    c->_hashMap->processFile(c->_arch);
    cerr << "soy hilo " << c->_h_id << " y pido el mutex " << endl;
    pthread_mutex_lock(&libre_mutex);
    cerr << "soy hilo " << c->_h_id << " y tengo el mutex " << endl;
    libre[c->_h_id] = true;
    pthread_mutex_unlock(&libre_mutex);
    cerr << "soy hilo " << c->_h_id << " y libere el  mutex " << endl;
    sem_post(&sem_c); //sumo un hilo terminado en el contador de terminados
    cerr << "soy hilo " << c->_h_id << " y te mande un signal " << endl;
    return NULL;
}

ConcurrentHashMap::ConcurrentHashMap() {
    for (int i = 0; i < maxLength; i++) {
        this->tabla[i] = new Lista<item>();
        pthread_mutex_init(&aai[i], NULL);
    }
}

ConcurrentHashMap::~ConcurrentHashMap() {
        /*for (int i = 0; i < maxLength; i++) {
            delete this->tabla[i];
            pthread_mutex_destroy(&aai[i]);
        }*/
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
        if(maxValue < maximosXFila[i]->second) {
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

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    ConcurrentHashMap* hashMap = new ConcurrentHashMap();
    unsigned int cant_archivos = archs.size();
    unsigned int cant_hilos = min(n, cant_archivos); //voy a crear a lo sumo cant achivos hilos por mas que n sea mayor
    cerr << "Hola Soy main, quizas me recuerden de otros TPs como OrgaII y AlgoIII" << endl;
    cerr << "soy main y voy a trabajar con " << cant_hilos << " hilos " << " y " << cant_archivos << " archivos" << endl;

    pthread_t hilo[cant_hilos];
    int h_id = 0;
    Cosa * cosa[cant_hilos];
    list<string>::iterator it = archs.begin();
    bool l[cant_hilos];
    libre = l;
    for (h_id = 0; h_id < cant_hilos; h_id++) {
        libre[h_id] = false;
    }

    sem_init(&sem_c, 0, -1);
    pthread_mutex_init(&libre_mutex, NULL);

    //PRIMERA ASIGNACION DE ARCHIVOS
    for (h_id = 0; h_id < cant_hilos; h_id++) {//pongo a correr en todos los hilos un archivo
        cerr << "soy main y voy a crear cosa para el archivo " << *it << " en hilo " << h_id << endl;
        cosa[h_id] = new Cosa(hashMap, *it, h_id);
        cerr << "soy main y voy a crear el hilo " << h_id << endl;
        pthread_create(&(hilo[h_id]), NULL, g, (void*) (cosa[h_id]));
        ++it;
    }//fin primera asignacion

    h_id = 0;

    while (it != archs.end()) {//ciclo para el resto de archivos con todos los hilos llenos
        cerr << "soy main y voy a trabajar con " << *it << endl;
        cerr << "soy main y me quedo esperando un signal" << endl;
        sem_wait(&sem_c);
        cerr << "soy main y alguien mando un signal" << endl;
        cerr << "soy main y pido el mutex" << endl;
        pthread_mutex_lock(&libre_mutex);
        cerr << "soy main y tengo el mutex" << endl;
        int iteracion = 0;
        while (!libre[h_id]) {//entre aca porque algun hilo avisó que estaba libre, busco alguno
            assert(iteracion < cant_hilos);
            iteracion++;
            cerr << "falso " << h_id << endl;
            h_id = (++h_id % cant_hilos);
        }
        cerr << "encontre libre a " << h_id << endl;
        libre[h_id] = false; //actualizo libre para poder liberar su mutex
        pthread_mutex_unlock(&libre_mutex); //libero el mutex de libre
        cerr << "soy main y liberé el mutex" << endl;
        //se que el hilo h_id terminó porque aparece en libre y liberó el mutex, y eso es lo ultimo que hace antes del return
        pthread_join(hilo[h_id], NULL); //espero a que haga el return a lo sumo
        cerr << "soy main y joinie el hilo " << h_id << endl;
        free(cosa[h_id]);
        cerr << "soy main y libere " << h_id << endl;
        cosa[h_id] = new Cosa(hashMap, *it, h_id);
        pthread_create(&(hilo[h_id]), NULL, g, (void*) (cosa[h_id]));
        ++it;
        h_id = (++h_id % cant_hilos);
    }

    for (h_id = 0; h_id < cant_hilos; h_id++) {
        pthread_join(hilo[h_id], NULL);
    }
    sem_destroy(&sem_c);
    return *hashMap;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    return make_pair("Hola", 3);
}
