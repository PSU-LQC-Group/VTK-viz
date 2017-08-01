ICC=/opt/rh/devtoolset-4/root/usr/bin/c++


VTKROOT=/gpfs/group/lqc3/default/VTK-static
# INCPATH=/gpfs/group/lqc3/default/VTK-viz/IO/ 
INCPATH=$(shell pwd)
# VTKLIB=-L${VTKROOT}/lib -lvtkIOImage
VTKINC=-I${VTKROOT}/include/vtk-8.1

CCFLAGS=-std=c++11
src=FreeFormatOneLine.cxx FreeFormatParser.cxx SplittableString.cxx Dataset.cxx VectorDomainDataset.cxx VectorDomain.cxx FerroDomain.cxx Data.cxx
hdr=$(src:.cxx=.h)
obj=$(src:.cxx=.o)

libGeneralIO.a:$(obj)
	ar rcs $@ $(obj)

%.o:%.cxx
	$(ICC) $(CCFLAGS) ${VTKINC} -I${INCPATH} -c $< 

.PHONY:clean
clean:
	/bin/rm -f *.o
	/bin/rm -f *.mod
	/bin/rm -f *.a
