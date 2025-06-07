CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

MAIN_EXEC = main.out
PART_EXEC = part.out
STORE_EXEC = store.out

MAIN_SRC = main.cpp
PART_SRC = part.cpp
STORE_SRC = store.cpp
LOGGER_SRC = logger.cpp

MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
PART_OBJ = $(PART_SRC:.cpp=.o)
STORE_OBJ = $(STORE_SRC:.cpp=.o)
LOGGER_OBJ = $(LOGGER_SRC:.cpp=.o)

all: $(MAIN_EXEC) $(PART_EXEC) $(STORE_EXEC)

$(MAIN_EXEC): $(MAIN_OBJ) $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(PART_EXEC): $(PART_OBJ) $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(STORE_EXEC): $(STORE_OBJ) $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	@read input && ./$(MAIN_EXEC) $$input
	rm -f $(MAIN_OBJ) $(PART_OBJ) $(STORE_OBJ) $(LOGGER_OBJ) $(MAIN_EXEC) $(PART_EXEC) $(STORE_EXEC)
	find . -type p -delete