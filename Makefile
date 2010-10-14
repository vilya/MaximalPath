# Note that you need to have the TBB environment variables set up before
# calling Make. You do this by sourcing the tbbvars.sh script that ships with
# TBB. For example, on 64-bit linux:
# 
#   source /opt/intel/tbb/3.0/bin/tbbvars.sh intel64

LIBS = -ltbb

CXXFLAGS = -g3 -Wall
CXX = g++


maximalpath: maximalpath.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) -o $@ $<


clean:
	rm -rf maximalpath *.o *.dSYM
