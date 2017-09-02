#include <iostream>
#include <cstdlib>
#include "ConcurrentHashMap.hpp"

using namespace std;

int main(int argc, char **argv) {
    ConcurrentHashMap h;
    list<string> l = {"corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4"};
    int i;

    h = ConcurrentHashMap::count_words(l);
    for (i = 0; i < 26; i++) {
        for (auto it = h.tabla[i]->CrearIt(); it.HaySiguiente(); it.Avanzar()) {
            auto t = it.Siguiente();
            cout << t.first << " " << t.second << endl;
        }
    }

    return 0;
}

