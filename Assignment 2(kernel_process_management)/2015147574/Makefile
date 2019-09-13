all : program1.o program2.o program3.o program4.o

program1.o : program1.cpp
	g++ program1.cpp -o program1
program2.o : program2.cpp
	g++ program2.cpp -lpthread -o program2
program3.o : program3.cpp
	g++ program3.cpp -lpthread -o program3
program4.o : program4.cpp
	g++ program4.cpp -o program4

clean :
	rm -rf program1 program1.o
	rm -rf program2 program2.o
	rm -rf program3 program3.o
	rm -rf program4 program4.o

