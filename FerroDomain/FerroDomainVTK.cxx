/*=========================================================================

Program:   Visualization Toolkit
Module:    FixedPointVolumeRayCastMapperCT.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// VTK includes
#include "vtkBoxWidget.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageResample.h"
#include "vtkMetaImageReader.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPlanes.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkXMLImageDataReader.h"
#include "vtkImageData.h"
#include "vtkDataSetMapper.h"
#include "vtkThreshold.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkCallbackCommand.h"
#include "vtkPointData.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"

#include <FreeFormatOneLine.h>
#include <FreeFormatParser.h>
#include <VectorDomainDataset.h>
#include <FerroDomain.h>
#include <Data.h>
#include <sstream>
#include <iostream>

#define VTI_FILETYPE 1
#define DAT_FILETYPE 2

#define VTK_CREATE(type, name) \
    vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace std;

//void KeypressCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData){
//    /* VTK_CREATE(vtkSmartVolumeMapper,mapper); */
//    vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
//
//    vtkSmartVolumeMapper* map=reinterpret_cast<vtkSmartVolumeMapper*>(clientData);
//
//    int number=(int)*iren->GetKeySym()-49;
//    int colNum=map->GetInput()->GetPointData()->GetNumberOfArrays();
//    cout << "Pressed:" << number<<colNum<<endl;
//    if (number<=colNum-1 && number >=0) {
//    map->SelectScalarArray(number);
//    map->Update();
//    }
//
//}

int main(int argc, char *argv[])
{
    // Parse the parameters

    double opacityWindow = 4096;
    double opacityLevel = 2048;
    int blendType = 0;
    int clip = 0;
    double reductionFactor = 1.0;
    double frameRate = 10.0;
    vector<string> fileName;
    vector<string> vtiFileName;
    vector<string> imageName;
    char **firstLevel[300],**secondLevel[300];

    bool independentComponents=true;
    bool fileFormatSTD=true;
    bool viewWindow = true;
    bool outputDomainAnalysis = false;
    double Value,R,G,B,Alpha;
    string shortFileName="",midFileName="",extension="";
    string choiceColor;
    int start=0,delta=0,end=0,count=1;
    double domainValue,domainAngle;
    ostringstream stringstream("");
    Data domainInfo;


    cout << "Start to read the strct.in file" << endl;
    FreeFormatParser structRead;
    structRead.setFilename("struct.in");
    structRead.parse();

    if (structRead.firstKeyExist("LSTDFORMAT")) {
        istringstream(structRead.getFirstLevel("LSTDFORMAT")[0]) >> boolalpha >>  fileFormatSTD;
        cout << setw(100) << right << "Choose to use the standard dat format " << boolalpha << fileFormatSTD<<structRead.getFirstLevel("LSTDFORMAT")[0] << endl;
    }else{
        cout << setw(100) << right << "No value of LSTDFORMAT is set, use the default value of " << fileFormatSTD << endl;
    }



    cout << argc << "argc" << endl;
    if (argc==1) {
        viewWindow=false;
        outputDomainAnalysis=false;

        if (structRead.firstKeyExist("FILENAME")) {
            shortFileName = structRead.getFirstLevel("FILENAME")[0]; 
            cout << setw(100) << right << "The common file name is " << shortFileName << endl;
        }else{
            cout << "!!!!!!!!!!!Error!!!!!!!!!!!" << endl;
            cout << "The FILENAME is not set yet" << endl;
            cout << "!!!!!!!!!!!Error!!!!!!!!!!!" << endl;
            exit(-1);
        }


        if (structRead.firstKeyExist("EXTENSION")) {
            extension = structRead.getFirstLevel("EXTENSION")[0];
            cout << setw(100) << right << "The data extension is " << extension << endl;
        }else{
            extension = "dat";
            cout << setw(100) << right << "No extension is specified, use the default one " << extension << endl;
        }


        if (fileFormatSTD) {
            if (structRead.firstKeyExist("START")) {
                istringstream(structRead.getFirstLevel("start")[0]) >> start ;
                cout << setw(100) << right << "The start time step is " << start << endl; 
            }else{
                cout << setw(100) << right << "No initial time step set, use the default value " << start << endl;
            }
            if (structRead.firstKeyExist("END")) {
                istringstream(structRead.getFirstLevel("end")[0]) >> end;
                cout << setw(100) << right << "The end time step is " << end << endl; 
            }else{
                cout << setw(100) << right << "No end time step set, use the default value " << end << endl;
            }
            if (structRead.firstKeyExist("DELTA")) {
                istringstream(structRead.getFirstLevel("delta")[0]) >> delta;
                cout << setw(100) << right << "The delta time step is " << delta << endl; 
            }else{
                delta = end - start;
                cout << setw(100) << right << "No delta time step is set, use the default value " << delta << endl; 
            }
            if (delta!=0) {
                count = (end-start)/delta;
            }else{
                count = 1;
            }
            for (int i = 0; i < count; i++) {

                stringstream.clear();
                stringstream.str("");
                stringstream << setw(8) << setfill('0') << i*delta+start ;

                midFileName = stringstream.str();
                fileName.push_back(shortFileName + "." + midFileName + "." + extension); 
            }

        }else{
            count=1;
            fileName.push_back(shortFileName + "." + extension);
            cout << "The fileName is " << fileName[0] << endl;
        }




    }else{
        viewWindow=true;
        fileName.push_back(argv[1]);
        outputDomainAnalysis=true;
        extension="dat";
        cout << "Visualize the file " << fileName[0] << endl;
    }









    if (structRead.firstKeyExist("LOOKUPTABLE")) {
        choiceColor = structRead.getFirstLevel("lookuptable")[0];
        cout << setw(100) << right << "Use the lookuptable " << choiceColor << endl; 
        if (structRead.secondKeyExist(choiceColor,"POINTADD")) {
            istringstream(structRead.getSecondLevel(choiceColor,"POINTADD")[0]) >> Value >>R >> G >> B >> Alpha; 
        }
    }else{
        cout << "No lookuptable specified, use the default one" << endl;
    }




    if (structRead.firstKeyExist("DOMAINVALUE")) {
        istringstream(structRead.getFirstLevel("domainValue")[0]) >> domainValue;
        cout << setw(100) << right << "The ferroelectric domain criteria of value is " << domainValue << endl;
    }else{
        domainValue = 0.1 ;
        cout << setw(100) << right << "No ferroelectric domain criteria of value is set, use the default value " << domainValue << endl;
    }

    if (structRead.firstKeyExist("DOMAINANGLE")) {
        istringstream(structRead.getFirstLevel("domainAngle")[0]) >> domainAngle;
        cout << setw(100) << right << "The ferroelectric domain criteria of angle is " << domainAngle << endl;
    }else{
        domainAngle = 180 ;
        cout << setw(100) << right << "No ferroelectric domain criteria of angle is set, use the default value " << domainAngle << endl;
    }


    cout << "Finished reading the struct.in file" << endl;


    FerroDomain ferroDomain;
    ferroDomain.setStandardValue(domainValue);
    ferroDomain.setStandardAngle(domainAngle);
    ferroDomain.printDomainInfo();


    cout << "Initialize vtk" << endl;

    // Create the renderer, render window and interactor
    /* vtkRenderer *renderer = vtkRenderer::New(); */
    VTK_CREATE(vtkRenderer,renderer);
    VTK_CREATE(vtkRenderWindow,renWin);

    // Connect it all. Note that funny arithematic on the
    // SetDesiredUpdateRate - the vtkRenderWindow divides it
    // allocated time across all renderers, and the renderer
    // divides it time across all props. If clip is
    // t[M#Arue then there are two props
    VTK_CREATE(vtkRenderWindowInteractor,iren);
    VTK_CREATE(vtkInteractorStyleTrackballCamera,style);
    // Read the data
    VTK_CREATE(vtkXMLImageDataReader,reader);
    VTK_CREATE(vtkImageData,input);

    VTK_CREATE(vtkThreshold,domainThreshold);
    VTK_CREATE(vtkDataSetMapper,domainMapper);
    VTK_CREATE(vtkSmoothPolyDataFilter,domainSmooth);
    VTK_CREATE(vtkPolyDataNormals,normalGenerator);
    VTK_CREATE(vtkDataSetSurfaceFilter,domainSurface);

    VTK_CREATE(vtkColorTransferFunction,color);
    VTK_CREATE(vtkPiecewiseFunction,compositeOpacity);

    VTK_CREATE(vtkWindowToImageFilter,windowToImageFilter);
    VTK_CREATE(vtkPNGWriter,writer);


    /* if (!viewWindow) { */
    if ( count > 1) {
        vector<string> header(ferroDomain.getDomainTypeCount()+1);
        header[0]="Index";
        for (int i = 0; i < ferroDomain.getDomainTypeCount(); i++) {
            header[i+1]=ferroDomain.getDomainTypeLabel(i); 
        }
        domainInfo.setFileName("Domain_Analytics.txt");
       domainInfo.initializeFile(header); 
    }
    for (int i = 0; i < count; i++) {

        cout <<setw(100) << right << "Start to read data file " << fileName[i] <<endl;
        if( extension == "vti" )
        {
        }
        else if ( extension == "dat" )
        {


            VectorDomainDataset dat;
            int datDim[4];
            dat.setDatFileName(fileName[i]);
            dat.readDatFile();
            dat.setOutputDomainAnalysis(outputDomainAnalysis);

            dat.outputVTIFile(ferroDomain,1);

            if (count >1) {
                vector<double> value(ferroDomain.getDomainTypeCount());
                for (int j = 0; j < ferroDomain.getDomainTypeCount(); j++) {
                    value[j]=dat.getDomainPercent(j);
                }
                domainInfo.outputWithIndex(i*delta+start,value);
            }


            vtiFileName.push_back(dat.getVTIFileName());
            imageName.push_back(dat.getLongFileName()+".png");
            /* dat.getDimension(datDim); */
            /* std::string vtiFile = dat.getVTIFileName(); */ 
            /* reader->SetFileName(vtiFile.c_str()); */
            /* reader->Update(); */
            /* std::cout << "dat readder successful"<<std::endl; */
        }
        else
        {
            cout << "Error! Not VTI or DAT!" << endl;
            exit(EXIT_FAILURE);
        }
    }


    vector<vtkActor *> actorDomain;
    for (int i = 0; i < 27; i++) {
        actorDomain.push_back(vtkActor::New());        
    }


    for (int m = 0; m < count; m++) {

        cout << "vtk part " << vtiFileName[m] << endl;

        reader->SetFileName(vtiFileName[m].c_str());
        reader->Update();
        input->ShallowCopy(reader->GetOutput());



        renWin->AddRenderer(renderer);
        renWin->Render();


        // Create our volume and mapper
        /* cout << reader->GetPointArrayStatus(reader->GetPointArrayName(1)) << endl;; */
        domainMapper->SetInputData( input );




    domainThreshold->SetInputConnection(reader->GetOutputPort());
    domainThreshold->AllScalarsOff();
    for (int i = 0; i < 27; i++) {
        domainThreshold->ThresholdBetween(i-0.5,i+0.5);
        domainThreshold->UpdateWholeExtent();
        domainSurface->SetInputConnection(domainThreshold->GetOutputPort());
        domainSurface->UpdateWholeExtent();
        domainSmooth->SetInputConnection(domainSurface->GetOutputPort());
        domainSmooth->SetNumberOfIterations(30);
        domainSmooth->SetRelaxationFactor(0.1);
        domainSmooth->FeatureEdgeSmoothingOff();
        domainSmooth->BoundarySmoothingOn();
        domainSmooth->Update();
        normalGenerator->SetInputData(domainSmooth->GetOutput());
        normalGenerator->ComputePointNormalsOn();
        normalGenerator->ComputeCellNormalsOn();
        normalGenerator->UpdateWholeExtent();
        domainMapper->SetInputConnection(normalGenerator->GetOutputPort());
        domainMapper->ScalarVisibilityOff();
        domainMapper->Update();
        actorDomain[i]->SetMapper(domainMapper);
        actorDomain[i]->GetProperty()->SetColor(ferroDomain.getDomainR(i),ferroDomain.getDomainG(i),ferroDomain.getDomainB(i));
        actorDomain[i]->GetProperty()->SetOpacity(1);
        actorDomain[i]->Modified();
        renderer->AddActor(actorDomain[i]); 
    }
    renderer->ResetCamera();
    renWin->Render();

    /* renWin->SetSize(600,600); */
    /* iren->Initialize(); */
    if (viewWindow) {

        iren->SetRenderWindow(renWin);
        iren->SetInteractorStyle(style);
        iren->Start();
    }else{
        cout << "Rendering off screen " << imageName[m] << endl;
        renWin->SetOffScreenRendering(1);
        windowToImageFilter->SetInput(renWin);
        windowToImageFilter->Update();
        writer->SetFileName(imageName[m].c_str());
        writer->SetInputConnection(windowToImageFilter->GetOutputPort());
        writer->Write();
    }
    }

    compositeOpacity->Delete();
    color->Delete();

    domainMapper->Delete();
    reader->Delete();
    renderer->Delete();
    renWin->Delete();
    iren->Delete();
    style->Delete();

    return 0;
}
