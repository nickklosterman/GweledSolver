CC=g++
CFLAGS=-c -Wall -O2 -o `GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs` -I/usr/include/GraphicsMagick/ -lX11 
LDFLAGS=
SOURCES=Gweled.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Gweled


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm -f main $(OBJECTS)
