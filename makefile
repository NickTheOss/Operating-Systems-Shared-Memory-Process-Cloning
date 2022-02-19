CPP=g++

objects=coordinator.o

SOURCE = coordinator.cpp

HEADER = coordinator.h 

OUT = coor

FLAGS = -g -c 

all : $(objects)
	$(CPP) -g $(objects) -o $(OUT) 

lsh.o : lsh.cpp
	$(CPP) $(FLAGS) os1.cpp

	
clean:
	rm -f os1 $(objects)
