#Makefile
CC = g++ 
CCFLAGS = -std=c++11 -g
LDFLAGS = -lncursesw

Snake : map_fin.o
	$(CC) $(CCFLAGS) -o snake map_fin.o $(LDFLAGS)

clean :
	rm -f *.o

%.o : %.cpp %.h
	$(CC) $(CCFLAGS) -c $<

%.o : %.cpp
	$(CC) $(CCFLAGS) -c $<

% : %.cpp
	$(CC) $(CCFLAGS) -o $@ $<
