#ifndef CONCURRENT_HASHMAP_H__
#define CONCURRENT_HASHMAP_H__

using namespace std;

#include <atomic>
#include <string>
#include "ListaAtomica.hpp"
#include <list>
#include <pthread.h>

typedef pair<string, unsigned int> item;

class ConcurrentHashMap {
private:

    static const unsigned int maxLength = 26;

    unsigned int hash(string s);
    pthread_mutex_t aai[maxLength];
    void processFile(string arch);
    static void* f(void * cosa);

    struct Cosa {

        Cosa(ConcurrentHashMap* hashMap, string arch) :
        _hashMap(hashMap),
        _arch(arch) {
            _conMutex = false;
        }

        Cosa(ConcurrentHashMap* hashMap, string arch, unsigned int h_id) :
        _hashMap(hashMap),
        _arch(arch),
        _h_id(h_id) {
            _conMutex = true;
        }

        ConcurrentHashMap* _hashMap;
        string _arch;
        unsigned int _h_id;
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
    static item maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs);


};

#endif /* CONCURRENT_HASHMAP_H__ */
