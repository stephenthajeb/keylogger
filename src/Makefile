`CXX = g++
CXXFLAGS = -fdiagnostics-color=always -g
SRC = main.cpp hook.cpp
OUT = clickme.exe
LIBS = -lwininet -luser32

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	del /f $(OUT)