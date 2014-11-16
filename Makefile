CXXFLAGS += -O2

SRC     = $(wildcard src/*.cpp)
OBJ     = $(SRC:.cpp=.o)
HEADER  = $(wildcard include/*.h) $(wildcard include/*.hpp)
INCLUDE = -Iinclude

LIB     = libpo.a
PROG    = po

.PHONY: program all clean

all: $(PROG)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) ${INCLUDE} -c -o $@ $<

lib: ${OBJ}
	ar rc ${LIB} ${OBJ}

$(PROG): $(OBJ) $(HEADER) lib
	$(CXX) $(LDFLAGS) ${INCLUDE} -static -o $@ main.cpp -L. -lpo

clean:
	rm -f ${OBJ} ${PROG} ${LIB} 

prof: ${SRC} ${HEADER}
	g++ ${CXXFLAGS} -g -pg ${SRC}
