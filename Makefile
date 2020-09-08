run:
	-make clean
	g++ -Wall -o output ./src/tokenlist.cpp ./src/parser.cpp ./src/main.cpp
	./output ./examples/largerscript.lak

clean:
	rm output
	rm *.o