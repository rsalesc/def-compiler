BDIR=build/

INCLUDES=-I.
MAIN_SOURCES=main.cpp parser.cpp ast.cpp
SOURCES=$(wildcard lexer/*.cpp) $(wildcard common/*.cpp) $(MAIN_SOURCES)
OBJECTS=$(addprefix $(BDIR), $(SOURCES:.cpp=.o))

BINARIES=a.out

CC=$(shell which g++)
CFLAGS=-std=c++11 -O2
LFLAGS=-lm

BISON=bison

OBJ_WILD=$(addprefix $(BDIR), %.o)
SRC_WILD=%.cpp

make: $(BDIR) $(dir $(OBJECTS)) $(BINARIES)

$(OBJ_WILD): $(SRC_WILD)
	$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

a.out: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS)

$(BDIR):
	mkdir -p $@

$(BDIR)%:
	mkdir -p $@

home: $(BDIR) parser $(dir $(OBJECTS)) $(BINARIES)
	@echo "done..."

package: make
	rm -rf mata61.zip
	zip -r mata61.zip main.cpp ast.cpp parser.cpp *.hpp lexer/ common/ tclap/ Makefile

clean:
	rm -f *.o $(OBJECTS) $(BINARIES) *.exe a.out
	rm -rf $(BDIR)
