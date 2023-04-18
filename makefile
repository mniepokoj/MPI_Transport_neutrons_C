CXX = mpic++
CXXFLAGS=-Wall -Wextra -Wpedantic -std=c++11 -Wno-error -I/home/mniepokoj/Downloads/sprng5/include -w
LDFLAGS=-L/home/mniepokoj/Downloads/sprng5/lib -lsprng

SRCDIR = src
BUILDDIR = build
OUTDIR = out

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))
TARGET = $(BUILDDIR)/program

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(OUTDIR)/results.dat

run: $(TARGET)
	mpirun -n 2 ./$(TARGET) 1000000 0.3 0.7 1
