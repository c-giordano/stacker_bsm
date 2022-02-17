CC=g++ -O3 -g -Wall -Wno-reorder -Wextra
CFLAGS= -Wl,--no-as-needed
CXXFLAGS = -std=gnu++17
LDFLAGS=`root-config --glibs --cflags`
BOOST = -lboost_system -lboost_filesystem
SOURCES= main.cc Styles/*.cc Helpers/src/*.cc Stacker/src/*.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE= stacker_exec

all: 
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(EXECUTABLE)
	
clean:
	rm -rf *o $(EXECUTABLE)
