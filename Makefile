BDIR=build/

INCLUDES=-I.
SOURCES=$(wildcard **/*.cpp)
OBJECTS=$(addprefix $(BDIR), $(SOURCES:.cpp=.o))
BINARIES=lex

CC=g++
CFLAGS=-std=c++11 -O2
LFLAGS=-lm

OBJ_WILD=$(addprefix $(BDIR), %.o)
SRC_WILD=%.cpp

make: $(BDIR) $(dir $(OBJECTS)) $(BINARIES)

$(OBJ_WILD): $(SRC_WILD)
	$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS)

$(BDIR):
	@mkdir -p $@

$(BDIR)%:
	@mkdir -p $@

clean:
	@echo $(dir $(OBJECTS))
	rm -f *.o $(OBJECTS) $(BINARIES)
	rm -rf $(BDIR)
