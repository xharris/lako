run:
	-make clean
	g++ -Wall -o output ./src/tokenlist.cpp ./src/parser.cpp ./src/main.cpp
	./output ./src/largerscript.lak

clean:
	rm output
	rm *.o