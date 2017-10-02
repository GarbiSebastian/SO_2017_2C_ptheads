.POSIX:
.SUFFIXES:
.SUFFIXES: .o .cpp

CXX = g++
CXXFLAGS = -std=c++11
LDLIBS = -lpthread
DEBUG = -g

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
	$(CXX) $(CXXFLAGS) -c $(DEBUG) $<

BIN = test-1 test-2 test-3 test-4 test-5 test-6 medidorCorpus medidorRandom generador
OBJ = ConcurrentHashMap.o

all: $(BIN)

$(BIN): ListaAtomica.hpp

test-1: $(OBJ) test-1.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-1.cpp $(OBJ) $(LDLIBS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) -o $@ test-1.cpp $(OBJ) $(LDLIBS)

test-1-run: test-1
	awk -f corpus.awk corpus | sort -nk 2 | tail -n 1 >corpus-max
	for i in 0 1 2 3 4; do ./test-1 $$((i + 1)) | diff -u - corpus-max; done
	rm -f corpus-max

test-2: $(OBJ) test-2.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-2.cpp $(OBJ) $(LDLIBS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) -o $@ test-2.cpp $(OBJ) $(LDLIBS)

test-2-run: test-2
	awk -f corpus.awk corpus | sort >corpus-post
	./test-2 | sort | diff -u - corpus-post
	rm -f corpus-post

test-3: $(OBJ) test-3.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-3.cpp $(OBJ) $(LDLIBS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) -o $@ test-3.cpp $(OBJ) $(LDLIBS)

test-3-run: test-3
	awk -f corpus.awk corpus | sort >corpus-post
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	for i in 0 1 2 3 4; do ./test-3 $$((i + 1)) 2> salida-test-3-$$((i + 1))hilo-error | sort | diff -u - corpus-post; done
	rm -f corpus-post corpus-[0-4]

seba: test-3
	./test-3 2 >/dev/null

test-4: $(OBJ) test-4.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-4.cpp $(OBJ) $(LDLIBS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) -o $@ test-4.cpp $(OBJ) $(LDLIBS)

test-4-run: test-4
	awk -f corpus.awk corpus | sort >corpus-post
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	./test-4 2> salida-test-4-error | sort | diff -u - corpus-post;
	rm -f corpus-post corpus-[0-4]

test-5: $(OBJ) test-5.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-5.cpp $(OBJ) $(LDLIBS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) -o $@ test-5.cpp $(OBJ) $(LDLIBS)

test-5-run: test-5
	awk -f corpus.awk corpus | sort -nk 2 | tail -n 1 >corpus-max
	cat corpus-max
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	for i in 0 1 2 3 4; do for j in 0 1 2 3 4; do \
		./test-5 $$((i + 1)) $$((j + 1)) 2>> tiempos-5 | diff -u - corpus-max; \
	done; done
	rm -f corpus-max corpus-[0-4]

test-6: $(OBJ) test-6.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-6.cpp $(OBJ) $(LDLIBS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEBUG) -o $@ test-6.cpp $(OBJ) $(LDLIBS)

test-6-run: test-6
	awk -f corpus.awk corpus | sort -nk 2 | tail -n 1 >corpus-max
#	cat corpus-max
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	for i in 0 1 2 3 4; do for j in 0 1 2 3 4; do \
		./test-6 $$((i + 1)) $$((j + 1)) 2>> tiempos-6 | diff -u - corpus-max; \
	done; done
	rm -f corpus-max corpus-[0-4]
	
medidorCorpus: $(OBJ) medidorCorpus.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ medidorCorpus.cpp $(OBJ) $(LDLIBS)

medidorCorpus-run: medidorCorpus
	rm -f tiempos
	echo "#hilos_archivos #hilos_max #cant_archivos maximum maximum2" > tiempos
	for i_archivos in $$(seq 1 5); do \
	for j_maximum in $$(seq 1 5); do \
	for k_lista in 1 2 3 4 5; do \
	./medidorCorpus $$((i_archivos*2)) $$j_maximum $$((k_lista*1000)) 2>> tiempos; \
	done; \
	done; \
	done;
	for i_archivos in $$(seq 1 20); do \
	for j_maximum in $$(seq 1 6); do \
	./medidorCorpus $$((i_archivos*20)) $$((j_maximum*4)) 1000 2>> tiempos; \
	done; \
	done;
	
medidorRandom: $(OBJ) medidorRandom.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ medidorRandom.cpp $(OBJ) $(LDLIBS)

medidorRandom-run: medidorRandom
	rm -f tiempos_random
	echo "#hilos_archivos #hilos_max #cant_archivos maximum maximum2" > tiempos_random
	for i_archivos in $$(seq 1 5); do \
	for j_maximum in $$(seq 1 5); do \
	for k_lista in 1 2 3 4 5; do \
	./medidorRandom $$((i_archivos*2)) $$j_maximum $$((k_lista*100)) 2>> tiempos_random; \
	done; \
	done; \
	done;
	for i_archivos in $$(seq 1 20); do \
	for j_maximum in $$(seq 1 6); do \
	./medidorRandom $$((i_archivos*20)) $$((j_maximum*4)) 100 2>> tiempos_random; \
	done; \
	done;

generador: $(OBJ) generador.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ generador.cpp $(OBJ) $(LDLIBS)

generador-run: generador
	rm -rf archivos;
	mkdir archivos;
	./generador 500 100 500 1 5 0
	
clean:
	rm -f $(BIN) $(OBJ)
	rm -f corpus-*
	rm -f tiempos-*
	rm -f salida-*

ConcurrentHashMap.o: ConcurrentHashMap.cpp

test-all-run: test-1-run test-2-run test-3-run test-4-run test-5-run test-6-run

