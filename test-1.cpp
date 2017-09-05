#include <iostream>
#include "ConcurrentHashMap.hpp"

using namespace std;

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "uso: " << argv[0] << " #threads" << endl;
        return 1;
    }
    ConcurrentHashMap h = ConcurrentHashMap::count_words("corpus");

    item dato = h.maximum(atoi(argv[1]));
    //    cout << "<" << dato.first << ", " << dato.second << ">" << endl;
    cout << dato.first << " " << dato.second << endl;

    return 0;
}

