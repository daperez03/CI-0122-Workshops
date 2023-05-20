mainObjets=src/Babuino.cpp src/main.cpp

all: $(mainObjets)
	g++ $(mainObjets) -o Babuino.run
	mkdir -p bin
	mv -f Babuino.run bin/

clean:
	rm -fr bin/