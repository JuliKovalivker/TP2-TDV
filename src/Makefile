CC = g++
CFLAGS = -std=c++17 -IClasses
SRC = test.cpp classes/GAPInstance.cpp classes/Asignacion.cpp classes/Solver.cpp utils/utils.cpp
OBJ = $(patsubst %.cpp,build/%.o,$(SRC))
EXEC = gap_simulator

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(EXEC)