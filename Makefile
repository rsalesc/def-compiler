BDIR=build/

BISON_FILE=decaf.y
BISON_TABS=decaf.tab.cpp

INCLUDES=-I.
SOURCES=$(wildcard **/*.cpp) $(BISON_TABS)
OBJECTS=$(addprefix $(BDIR), $(SOURCES:.cpp=.o))

BINARIES=a.out

CC=$(shell which g++)
CFLAGS=-std=c++11 -O2 -g3
LFLAGS=-lm

BISON=bison

OBJ_WILD=$(addprefix $(BDIR), %.o)
SRC_WILD=%.cpp

make: $(BDIR) $(dir $(OBJECTS)) $(BINARIES)

$(OBJ_WILD): $(SRC_WILD)
	$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

a.out: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS)

parser:
	bison $(BISON_FILE) -o $(BISON_TABS)

$(BDIR):
	mkdir -p $@

$(BDIR)%:
	mkdir -p $@

home: $(BDIR) parser $(dir $(OBJECTS)) $(BINARIES)
	@echo "done..."

clean:
	rm -f *.o $(OBJECTS) $(BINARIES) *.exe a.out
	rm -rf $(BDIR)
