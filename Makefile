CXXFLAGS += -O2

INCLUDE = include
SRC     = $(wildcard src/*.cpp)
OBJ     = $(SRC:.cpp=.o)
HEADER  = $(wildcard $(INCLUDE)/*.h) $(wildcard $(INCLUDE)/*.hpp)

LIB     = libpo.a
PROG    = po

.PHONY: program all clean

all: $(PROG)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE) -c -o $@ $<

lib: $(OBJ)
	ar rc $(LIB) $(OBJ)

$(PROG): $(OBJ) $(HEADER) lib
	$(CXX) $(LDFLAGS) -I$(INCLUDE) -static -o $@ main.cpp -L. -lpo

clean:
	rm -f $(OBJ) $(PROG) $(LIB) prof

prof: $(SRC) $(HEADER) lib
	$(CXX) $(LDFLAGS) -g -pg -I$(INCLUDE) -static -o $@ main.cpp -L. -lpo
