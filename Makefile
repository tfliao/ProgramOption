CXXFLAGS += -O2

INCLUDE = include
SRC     = $(wildcard src/*.cpp)
OBJ     = $(SRC:.cpp=.o)
HEADER  = $(wildcard $(INCLUDE)/*.h) $(wildcard $(INCLUDE)/*.hpp)

LIB     = libpo.a
PROG    = sample

INCLUDE_OPT = $(addprefix -I, $(INCLUDE))

.PHONY: all clean

all: $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_OPT) -c -o $@ $<

$(LIB): $(OBJ)
	ar rc $@ $(OBJ)

$(PROG): $(OBJ) $(HEADER) $(LIB)
	$(CXX) $(LDFLAGS) -static -o $@ main.cpp $(INCLUDE_OPT) $(LIB)

clean:
	rm -f $(OBJ) $(PROG) $(LIB)

