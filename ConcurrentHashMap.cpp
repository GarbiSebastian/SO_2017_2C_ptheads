using namespace std;

#include "ConcurrentHashMap.hpp"
#include <iostream>

unsigned int ConcurrentHashMap::hash(string s){
	return (int)(s.at(0)) - 97;
}

ConcurrentHashMap::ConcurrentHashMap(){
	for(int i = 0; i< 26;i++){
		this->tabla[i] = new Lista<item>();
	}
}
void ConcurrentHashMap::addAndInc(string key){
	item t;
	bool encontre = false;
	int hashKey = this->hash(key);
	
	auto it = this->tabla[hashKey]->CrearIt();
	while(it.HaySiguiente() && !encontre){
		t = it.Siguiente();
		encontre = t.first == key;
		it.Avanzar();
	}
	if(encontre){
		t.second++;
	}else{
		this->tabla[hashKey]->push_front(make_pair(key,1));
	}
}

bool ConcurrentHashMap::member(string key){
	return true;
}

item ConcurrentHashMap::maximum(unsigned int nt){
	return make_pair("Hola",3);
}

ConcurrentHashMap ConcurrentHashMap::count_words(string arch){
	ConcurrentHashMap a;
	FILE * fd = fopen(arch.c_str(),"r");
	
	fclose(fd);
	return a;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs){
	ConcurrentHashMap a;
	return a;
}
ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs){
	ConcurrentHashMap a;
	return a;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs){
	return make_pair("Hola",3);
}
