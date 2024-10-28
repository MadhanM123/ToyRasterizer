CXX = g++
CXXFLAGS =
LDFLAGS =
LIBS = -lm

TARGET = main

OBJECTS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(CXX) -Wall $(CXXFLAGS) -c $(CFLAGS) $< -o $@


clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f *.tga