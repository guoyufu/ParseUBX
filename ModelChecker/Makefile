all: modelcheck

modelcheck: main.o ModelChecker.o LibUBX.o LibNMEA.o ParseUBX.o
	g++ main.o ModelChecker.o LibUBX.o LibNMEA.o ParseUBX.o -o modelcheck
	
main.o: main.cpp
	g++ -c main.cpp
	
ModelChecker.o: ModelChecker.cpp
	g++ -c ModelChecker.cpp

LibUBX.o: ../ParseUBX/LibUBX.cpp
	g++ -c ../ParseUBX/LibUBX.cpp
	
LibNMEA.o: ../ParseUBX/LibNMEA.cpp
	g++ -c ../ParseUBX/LibNMEA.cpp

ParseUBX.o: ../ParseUBX/ParseUBX.cpp
	g++ -c ../ParseUBX/ParseUBX.cpp