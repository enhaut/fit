CXX=icpc -std=c++11 -g -xhost -qopt-report=5 -qopt-zmm-usage=high -qopt-report-phase=vec,par -vecabi=cmdtarget
CXXFLAGS=-O3 -qopenmp-simd -qopenmp
TARGET=lab4

all:
	$(CXX) $(CXXFLAGS) $(TARGET).cpp -o lab4

run:
	./lab4


clean:
	rm -f lab4 *.optrpt
