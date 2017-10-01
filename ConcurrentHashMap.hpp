#ifndef CONCURRENT_HASHMAP_H__
#define CONCURRENT_HASHMAP_H__

using namespace std;

#include <atomic>
#include <string>
#include "ListaAtomica.hpp"
#include <list>
#include <pthread.h>
#include <semaphore.h>

typedef pair<string, unsigned int> item;

class ConcurrentHashMap {
protected:
    static const unsigned int maxLength = 26;
    pthread_rwlock_t aai[maxLength];
    
    unsigned int hash(string s);
    void procesarArchivo(string arch);
    static void* hiloProcesarArchivo(void * parametro);
    static void* calcularMaximos(void * parametro);
    static void* procesarListaDeArchivos(void * parametro);
    static void* crear_hashMaps(void* parametro);
    void addAndInc(string key,unsigned int n);

    struct ParametroMaximum {

        ParametroMaximum(ConcurrentHashMap* _hashMap) :
        _hashMap(_hashMap) {
            this->_proximaLetra = 0;
        }
        ConcurrentHashMap* _hashMap;
        std::atomic_uint _proximaLetra;
    };

    struct ParametroProcesarArchivo {

        ParametroProcesarArchivo(ConcurrentHashMap* hashMap, string arch) :
        _hashMap(hashMap),
        _arch(arch) {
        }
        ConcurrentHashMap* _hashMap;
        string _arch;
    };

    struct ParametroProcesarListaDeArchivos {

        ParametroProcesarListaDeArchivos(ConcurrentHashMap* hashMap, list<string>::iterator* it, list<string>::iterator* end) :
        _hashMap(hashMap),
        _it(it),
        _end(end) {
        }
        ConcurrentHashMap* _hashMap;
        list<string>::iterator* _it;
        list<string>::iterator* _end;
    };

    struct ParametroCrearHashMaps {
        ParametroCrearHashMaps(Lista<ConcurrentHashMap>* hashMaps, list<string>::iterator* it, list<string>::iterator* end) :
        _hashMaps(hashMaps),
        _it(it),
        _end(end) {
        }
        Lista<ConcurrentHashMap>* _hashMaps;
        list<string>::iterator* _it;
        list<string>::iterator* _end;
    };

public:
    Lista<item>* tabla[26];
    ConcurrentHashMap();
    ~ConcurrentHashMap();
    void addAndInc(string key);
    bool member(string key);
    item maximum(unsigned int nt);
    static ConcurrentHashMap count_words(string arch);
    static ConcurrentHashMap count_words(list<string> archs);
    static ConcurrentHashMap count_words(unsigned int n, list<string> archs);
    static item maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string> archs);
    static item maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs);

};

#endif /* CONCURRENT_HASHMAP_H__ */
