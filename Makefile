CXXFLAGS += -O2

INCLUDE = include
SRC     = $(wildcard src/*.cpp)
OBJ     = $(SRC:.cpp=.o)
HEADER  = $(wildcard $(INCLUDE)/*.h) $(wildcard $(INCLUDE)/*.hpp)

LIB     = libpo.a
PROG    = sample

LINK_PATH = .
LINK_LIBS = po

INCLUDE_OPT = $(addprefix -I, $(INCLUDE))
LINK_OPT = $(addprefix -L, $(LINK_PATH)) $(addprefix -l, $(LINK_LIBS))


.PHONY: program all clean

all: $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_OPT) -c -o $@ $<

$(LIB): $(OBJ)
	ar rc $@ $(OBJ)

$(PROG): $(OBJ) $(HEADER) $(LIB)
	$(CXX) $(LDFLAGS) -static -o $@ main.cpp $(LINK_OPT) $(INCLUDE_OPT)

clean:
	rm -f $(OBJ) $(PROG) $(LIB)

