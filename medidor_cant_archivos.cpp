#include <iostream>
#include <cstdlib>
#include "ConcurrentHashMap.hpp"
#include <time.h>

using namespace std;

int main(int argc, char **argv) {
    
    if (argc != 3) {
        cerr << "uso: " << argv[0] << " #tarchivos #tmaximum" << endl;
        return 1;
    }
    

    //fstream tiemp ("exp_cant_archivos_caso1.txt", ios::in | ios::out);
    pair<string, unsigned int> p;
    list<string> l;
    int contador = 0;

    for(unsigned int j = 0; j < 25; j++){

        for(unsigned int i = 0; i < 200; i++){
            l.push_front("corpus");
        }
        contador = contador + 200;

		clock_t inicio1 = clock();
		p = ConcurrentHashMap::maximum(atoi(argv[1]), atoi(argv[2]), l);
		//cout << p.first << " " << p.second << endl;
		clock_t fin1 = clock();
		clock_t tiempo1 = fin1 - inicio1;
		
		clock_t inicio2 = clock();
		p = ConcurrentHashMap::maximum2(atoi(argv[1]), atoi(argv[2]), l);
		//cout << p.first << " " << p.second << endl;
		clock_t fin2 = clock();
		clock_t tiempo2 = fin2 - inicio2;
		
		cerr
				<< argv[1] << " "
				<< argv[2] << " "
				<< contador << " "
				<< ((float) tiempo1) / CLOCKS_PER_SEC << " "
				<< ((float) tiempo2) / CLOCKS_PER_SEC << " "
				<< endl;
		
    }
    return 0;
}
