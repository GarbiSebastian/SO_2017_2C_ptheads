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
private:

    static const unsigned int maxLength = 26;

    unsigned int hash(string s);
//    pthread_mutex_t aai[maxLength];
    pthread_rwlock_t aai[maxLength];
    void procesarArchivo(string arch);
    static void* f(void * cosa);
    static void* g(void * cosa);
    static void* crear_hashMaps(void* c);
    void add_hashMaps(item* p);

    struct Cosa {

        Cosa(ConcurrentHashMap* hashMap, string arch) :
        _hashMap(hashMap),
        _arch(arch) {
            _conMutex = false;
        }

        /*Cosa(ConcurrentHashMap* hashMap, string arch, unsigned int h_id) :
        _hashMap(hashMap),
        _arch(arch),
        _h_id(h_id) {
            _conMutex = true;
        }*/
        
        Cosa(ConcurrentHashMap* hashMap, list<string>::iterator* it,list<string>::iterator* end ) :
        _hashMap(hashMap),
        _it(it),
        _end(end)
        {
            _conMutex = false;
        }
       
        ConcurrentHashMap* _hashMap;
        list<string>::iterator* _it;
        list<string>::iterator* _end;
        
        string _arch;
        //unsigned int _h_id;
        bool _conMutex;
    };
	
    struct Cosa2 {

        Cosa2(Lista<ConcurrentHashMap>* hashMaps, list<string>::iterator* it,list<string>::iterator* end ) :
        _hashMaps(hashMaps),
        _it(it),
        _end(end)
        {
            _conMutex = false;
        }

        Lista<ConcurrentHashMap>* _hashMaps;
        list<string>::iterator* _it;
        list<string>::iterator* _end;

        bool _conMutex;
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
    //static item maximum(unsigned int p_maximos, list<string> archs);
    static item maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string> archs);
    static item maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs);

};

#endif /* CONCURRENT_HASHMAP_H__ */
