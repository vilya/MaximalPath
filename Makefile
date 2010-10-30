# Note that you need to have the TBB environment variables set up before
# calling Make. You do this by sourcing the tbbvars.sh script that ships with
# TBB. For example, on 64-bit linux:
#
#   source /opt/intel/tbb/3.0/bin/tbbvars.sh intel64

LIBS = -ltbb

CXXFLAGS = -O3 -funroll-loops
CXX = icpc

CPP_RESULTS  = $(patsubst testdata/%-graph.txt,outputs/cpp/%-output.txt,$(wildcard testdata/*-graph.txt))
PY_RESULTS   = $(patsubst testdata/%-graph.txt,outputs/py/%-output.txt,$(wildcard testdata/*-graph.txt))


all: maximalpath


maximalpath: maximalpath.cpp maximalpath.h
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $<


.PRECIOUS: $(CPP_RESULTS) $(PY_RESULTS)
.PHONY: cpp-run py-run diff-run
cpp-run: $(CPP_RESULTS)
py-run: $(PY_RESULTS)


outputs/py/%-output.txt: testdata/%-graph.txt testdata/%-nodes.txt
	./maximalpath.py $^ > $@


outputs/cpp/%-output.txt: testdata/%-graph.txt testdata/%-nodes.txt
	./maximalpath $^ > $@


clean:
	rm -rf maximalpath *.o *.dSYM


outclean:
	rm -f outputs/*/*
