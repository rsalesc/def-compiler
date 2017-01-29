BDIR=build/

INCLUDES=-I.
SOURCES=$(wildcard **/*.cpp)
OBJECTS=$(addprefix $(BDIR), $(SOURCES:.cpp=.o))
BINARIES=a.out

CC=$(shell which g++)
CFLAGS=-std=c++11 -O2 -g3
LFLAGS=-lm

OBJ_WILD=$(addprefix $(BDIR), %.o)
SRC_WILD=%.cpp

make: $(BDIR) $(dir $(OBJECTS)) $(BINARIES)

$(OBJ_WILD): $(SRC_WILD)
	$(CC) $(INCLUDES) -c $< -o $@ $(CFLAGS)

a.out: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LFLAGS)

$(BDIR):
	@mkdir -p $@

$(BDIR)%:
	@mkdir -p $@

clean:
	@echo $(CC)
	rm -f *.o $(OBJECTS) $(BINARIES) *.exe a.out
	rm -rf $(BDIR)
