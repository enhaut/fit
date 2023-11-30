CXX=icpc -std=c++11 -g -xhost -qopt-report=5 -qopt-zmm-usage=high -qopt-report-phase=vec,par -vecabi=cmdtarget
CXXFLAGS=-O3 -qopenmp-simd -qopenmp
TARGET=lab5

all:
	$(CXX) $(CXXFLAGS) $(TARGET).cpp -o lab5

run:
	./lab5

run1:
	OMP_NUM_THREADS=1 ./lab5
run2:
	OMP_NUM_THREADS=2 ./lab5
run4:
	OMP_NUM_THREADS=4 ./lab5
run8:
	OMP_NUM_THREADS=8 ./lab5
run16:
	OMP_NUM_THREADS=16 ./lab5
run24:
	OMP_NUM_THREADS=24 ./lab5
run36:
	OMP_NUM_THREADS=36 ./lab5

bench:
	OMP_NUM_THREADS=1 ./lab5
	OMP_NUM_THREADS=2 ./lab5
	OMP_NUM_THREADS=4 ./lab5
	OMP_NUM_THREADS=8 ./lab5
	OMP_NUM_THREADS=16 ./lab5
	OMP_NUM_THREADS=24 ./lab5
	OMP_NUM_THREADS=36 ./lab5


clean:
	rm -f lab5 *.optrpt
