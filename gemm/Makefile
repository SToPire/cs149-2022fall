CXX=g++ -m64
CXXFLAGS=-I../common -Iobjs/ -O2 -Wall
ISPC=ispc
# note: requires AVX2
ISPCFLAGS=-O3 --target=avx2-i32x8 --arch=x86-64

APP_NAME=gemm
OBJDIR=objs
COMMONDIR=../common


TASKSYS_CXX=$(COMMONDIR)/tasksys.cpp
TASKSYS_LIB=-lpthread
TASKSYS_OBJ=$(addprefix $(OBJDIR)/, $(subst $(COMMONDIR)/,, $(TASKSYS_CXX:.cpp=.o)))

MKL_CXX=-DMKL_ILP64 -I$(MKLROOT)/include
MKL_LIB=-Wl,--start-group $(MKLROOT)/lib/intel64/libmkl_intel_ilp64.a \
	$(MKLROOT)/lib/intel64/libmkl_gnu_thread.a \
	$(MKLROOT)/lib/intel64/libmkl_core.a \
	-Wl,--end-group -lgomp -lpthread -lm -ldl

default: $(APP_NAME)

.PHONY: dirs clean

dirs:
		/bin/mkdir -p $(OBJDIR)/

clean:
		/bin/rm -rf $(OBJDIR) *.ppm *~ $(APP_NAME)

OBJS=$(OBJDIR)/main.o $(OBJDIR)/gemm.o $(OBJDIR)/gemm_ispc.o $(TASKSYS_OBJ) 

$(APP_NAME): dirs $(OBJS)
		$(CXX) $(CXXFLAGS) $(MKL_CXX) -o $@ $(OBJS) ref_gemm_ispc.a -lm $(MKL_LIB) -lm $(TASKSYS_LIB)

$(OBJDIR)/%.o: %.cpp
		$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/%.o: $(COMMONDIR)/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/main.o: $(OBJDIR)/$(APP_NAME)_ispc.h $(COMMONDIR)/CycleTimer.h

$(OBJDIR)/%_ispc.h $(OBJDIR)//%_ispc.o: %.ispc
		$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h
