CC = g++
CXXFLAGS = -std=c++11 -Wall
CFLAGS = -lsndfile
OUTPUT = VoiceFilters.out

all: $(OUTPUT)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OUTPUT): main.cpp
	$(CC) main.cpp $(CFLAGS) -o $(OUTPUT)

.PHONY: clean

clean:
	rm -f $(OUTPUT)