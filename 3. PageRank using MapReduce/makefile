all: part1 part2 part3

part1: mr-pr-cpp.cpp 
	mpic++ -std=c++11 -I include/detail -I include  mr-pr-cpp.cpp -lboost_system -lpthread -lboost_iostreams -lboost_filesystem -o mr-pr-cpp.o

part2: mr-pr-mpi.cpp 
	mpic++ -std=c++11 -I include/detail -I include  mr-pr-mpi.cpp -lboost_system -lpthread -lboost_iostreams -lboost_filesystem -o mr-pr-mpi.o

part3: mr-pr-mpi-base.cpp 
	mpic++ -std=c++11 -I include/detail -I include  mr-pr-mpi-base.cpp -lboost_system -lpthread -lboost_iostreams -lboost_filesystem -o mr-pr-mpi-base.o

clean:
	rm -f *.o
