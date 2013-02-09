INCLUDE=-I/usr/include/GraphicsMagick/ 
CC=g++
CFLAGS=-O2 -o ${INCLUDES}
LIBS=`GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs` -lX11 
LDFLAGS=
SOURCES=Gweled.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Gweled


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm -f core $(OBJECTS)
