INCDIR = $(GARFIELD_HOME)/Include
HEEDDIR = $(GARFIELD_HOME)/Heed
LIBDIR = $(GARFIELD_HOME)/lib
PYTHONPATH=..:$PYTHONPATH

# Compiler flags
CFLAGS = -Wall -Wextra -Wno-long-long \
	`root-config --cflags` \
	 -fno-common -c \
	-I$(INCDIR) -I$(HEEDDIR)

LDFLAGS = `root-config --glibs` -lGeom -lgfortran -lm \
	-L$(LIBDIR) -lGarfield

all: avalanche geometry/geometry/field.result geometry/geometry/mesh.elements geometry/geometry.msh geometry/geometry/field_weight.result

avalanche: avalanche.cpp ../simulation.conf
	cog.py -r avalanche.cpp
	$(CXX) $(CFLAGS) avalanche.cpp
	$(CXX) -o avalanche avalanche.o $(LDFLAGS)
	rm avalanche.o

geometry/geometry/field.result: geometry/geometry/mesh.elements geometry/calculate_field.sif
	cog.py -r geometry/calculate_field.sif
	cd geometry; ElmerSolver calculate_field.sif

geometry/geometry/field_weight.result: geometry/calculate_field_weight.sif
	cog.py -r geometry/calculate_field_weight.sif
	cd geometry; ElmerSolver calculate_field_weight.sif

geometry/geometry/mesh.elements: geometry/geometry.msh
	cd geometry; ElmerGrid 14 2 geometry.msh -autoclean

geometry/geometry.msh: geometry/geometry.step
	cd geometry; gmsh geometry.step -3 -order 2 -clmax 0.015

clean-cog:
	cog.py -x -r avalanche.cpp
	cog.py -x -r geometry/calculate_field.sif

clean:
	make clean-cog
	rm geometry/geometry/field.result 
	
