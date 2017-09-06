#include <iostream>
#include <cstdlib>
#include "ConcurrentHashMap.hpp"

using namespace std;

int main(int argc, char **argv) {
    clock_t inicio = clock();
    pair<string, unsigned int> p;
    list<string> l = {"corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4"};

    if (argc != 3) {
        cerr << "uso: " << argv[0] << " #tarchivos #tmaximum" << endl;
        return 1;
    }
    p = ConcurrentHashMap::maximum2(atoi(argv[1]), atoi(argv[2]), l);
    cout << p.first << " " << p.second << endl;
    clock_t fin = clock();
    clock_t tiempo = fin - inicio;
    cerr << "inicio: " << inicio << " "
            << "fin: " << fin << " "
            << "transcurrido: " << ((float) tiempo) / CLOCKS_PER_SEC
            << endl;
    return 0;
}

