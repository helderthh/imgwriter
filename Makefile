CXX = g++
CXXFLAGS += -std=c++11

CPPFLAGS += `pkg-config --cflags opencv4`
LDFLAGS += -I/usr/local/1.67.0_1 -lboost_system `pkg-config --libs opencv4`

INCLUDE_PATH = include
SRC_PATH = src

CPPFLAGS += -I$(INCLUDE_PATH)

all: imgwriter

imgwriter: $(INCLUDE_PATH)/base64.o
	$(CXX) $(CXXFLAGS) -o $@ $^ main.cpp $(LDFLAGS) $(CPPFLAGS)

$(INCLUDE_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(INCLUDE_PATH) -c -o $@ $<

run:
	./imgwriter 127.0.0.1 8009 1

clear:
	rm -f $(INCLUDE_PATH)/*.o *.o imgwriter
