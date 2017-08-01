#include <FerroDomain.h>
#include <Dataset.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>


using namespace std;

#ifndef VECTORDOMAINDATASET_H
#define VECTORDOMAINDATASET_H

class VectorDomainDataset:public Dataset{
    public:
        VectorDomainDataset();
        VectorDomainDataset(string);
        ~VectorDomainDataset();

//        void setDatFileName(string);
//        string getDatFileName();
//        string getVTKFileName();
//        string getVTIFileName();
//        void setVTKFileName(string);
//        void setVTIFileName(string);
//        /* void readVTKFile(); */
//        void outputVTKFile();
//        void outputVTIFile();
//        void setDimension(int,int,int,int);
//        double **** getData();
//        int * getDimension(int *dim);
//
//
          void readDatFile();
          void outputVTIFile(VectorDomain,int);
          void processDomain(VectorDomain,int);
    private:
//        string datFileName="";
//        string vtkFileName="";
//        string vtiFileName="";
//        string fileName;
//        double **** data;
//        int x=0,y=0,z=0,col=0;
//
//
          double * domainPercent;
          int *** domainLabel;
          int domainCol;
        
};
#endif
