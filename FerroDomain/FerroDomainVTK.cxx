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
#include "vtkUnstructuredGrid.h"

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
    double R,G,B,Alpha;
    int Value;
    string shortFileName="",midFileName="",extension="";
    string choiceColor;
    int start=0,delta=0,end=0,count=1;
    double domainValue,domainAngle;
    double focalX,focalY,focalZ;
    double positionX,positionY,positionZ;
    double upX,upY,upZ;
    ostringstream stringstream("");
    Data domainInfo;
    string cameraName;
    int datDim[4];
    int printWidth=80;
    double ** lookUpTable;
    bool * domainVisibility; 
    int domainTypeCount=0;
    int colorCount=0;
    bool controlFileExist=false;
    bool visibleAll=true;
    int number;
    int windowX=600,windowY=600;

    /* First setup to read the struct.in file 
     * The LSTDFORMAT, FILENAME, EXTENSION, START, DELTA, END
     * DOMAINVALUE, DOMAINANGLE tags can be read first
     *
     * */ 

    FreeFormatParser structRead;
    structRead.setFilename("visual.in");
    controlFileExist=structRead.parse();

    if (controlFileExist) {
        cout << "Start to read the visual.in file" << endl;
    }else if(argc>1){
        cout << "Get parameter from command line" << endl;
    }else{
        cout << "Either the visual.in file need to be given or a arguement is needed by the program" << endl;
        exit(-1);
    }


    if (structRead.firstKeyExist("LSTDFORMAT")) {
        istringstream(structRead.getFirstLevel("LSTDFORMAT")[0]) >> boolalpha >>  fileFormatSTD;
        cout << setw(printWidth) << right << "Choose to use the standard dat format " << boolalpha << fileFormatSTD << endl;
    }else{
        cout << setw(printWidth) << right << "No value of LSTDFORMAT is set, use the default value of " << fileFormatSTD << endl;
    }

    if (structRead.firstKeyExist("WINSIZE")) {
        istringstream(structRead.getFirstLevel("WINSIZE")[0]) >> windowX >>  windowY;
        cout << setw(printWidth) << right << "The render window size is  " << windowX << " "<<windowY << endl;
    }else{
        cout << setw(printWidth) << right << "No render window size is set, use the default value of " << windowX << " " << windowY << endl;
    }





    /* cout << argc << "argc" << endl; */
    /* If no filename is passed using arguement, 
     * then read FILENAME, EXTENSION, START, DELTA, END
     * from the struct.in file*/

    if (argc==1) {
        viewWindow=false;
        outputDomainAnalysis=false;

        if (structRead.firstKeyExist("FILENAME")) {
            shortFileName = structRead.getFirstLevel("FILENAME")[0]; 
            cout << setw(printWidth) << right << "The common file name is " << shortFileName << endl;
        }else{
            cout << "!!!!!!!!!!!Error!!!!!!!!!!!" << endl;
            cout << "The FILENAME is not set yet" << endl;
            cout << "!!!!!!!!!!!Error!!!!!!!!!!!" << endl;
            exit(-1);
        }


        if (structRead.firstKeyExist("EXTENSION")) {
            extension = structRead.getFirstLevel("EXTENSION")[0];
            cout << setw(printWidth) << right << "The data extension is " << extension << endl;
        }else{
            extension = "dat";
            cout << setw(printWidth) << right << "No extension is specified, use the default one " << extension << endl;
        }


        if (fileFormatSTD) {
            if (structRead.firstKeyExist("START")) {
                istringstream(structRead.getFirstLevel("start")[0]) >> start ;
                cout << setw(printWidth) << right << "The start time step is " << start << endl; 
            }else{
                cout << setw(printWidth) << right << "No initial time step set, use the default value " << start << endl;
            }
            if (structRead.firstKeyExist("END")) {
                istringstream(structRead.getFirstLevel("end")[0]) >> end;
                cout << setw(printWidth) << right << "The end time step is " << end << endl; 
            }else{
                cout << setw(printWidth) << right << "No end time step set, use the default value " << end << endl;
            }
            if (structRead.firstKeyExist("DELTA")) {
                istringstream(structRead.getFirstLevel("delta")[0]) >> delta;
                cout << setw(printWidth) << right << "The delta time step is " << delta << endl; 
            }else{
                delta = end - start;
                cout << setw(printWidth) << right << "No delta time step is set, use the default value " << delta << endl; 
            }
            if (delta!=0) {
                count = (end-start)/delta + 1;
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
            cout << setw(printWidth) << right << "The fileName is " << fileName[0] << endl;
        }

        cout << setw(printWidth) << right << "Numbers of file to be visualized " << count << endl;


    }else{
        viewWindow=true;
        fileName.push_back(argv[1]);
        outputDomainAnalysis=true;
        extension=fileName[0].substr(fileName[0].find_last_of(".")+1);
        cout << setw(printWidth) << "Visualize the file " << fileName[0] << endl;
        cout << setw(printWidth) << "File extension is " << extension << endl;
    }

    if (structRead.firstKeyExist("DOMAINVALUE")) {
        istringstream(structRead.getFirstLevel("domainValue")[0]) >> domainValue;
        cout << setw(printWidth) << right << "The ferroelectric domain criteria of value is " << domainValue << endl;
    }else{
        domainValue = 0.1 ;
        cout << setw(printWidth) << right << "No ferroelectric domain criteria of value is set, use the default value " << domainValue << endl;
    }

    if (structRead.firstKeyExist("DOMAINANGLE")) {
        istringstream(structRead.getFirstLevel("domainAngle")[0]) >> domainAngle;
        cout << setw(printWidth) << right << "The ferroelectric domain criteria of angle is " << domainAngle << endl;
    }else{
        domainAngle = 180 ;
        cout << setw(printWidth) << right << "No ferroelectric domain criteria of angle is set, use the default value " << domainAngle << endl;
    }

    if (structRead.firstKeyExist("VISIBLEALL")) {
        istringstream(structRead.getFirstLevel("VISIBLEALL")[0])>>boolalpha >> visibleAll;

        cout << setw(printWidth) << right << "All domains are visible " << boolalpha << visibleAll << endl ;
    }else{
        visibleAll=true;
        cout << setw(printWidth) << right << "Use default value, All domains are visible " << boolalpha << visibleAll << endl ;
    }



    cout << "Finished reading the visual.in file" << endl;


    FerroDomain domain;
    domain.setStandardValue(domainValue);
    domain.setStandardAngle(domainAngle);
    domain.printDomainInfo();
    domainTypeCount = domain.getDomainTypeCount();

    lookUpTable = new double*[domainTypeCount];
    domainVisibility = new bool[domainTypeCount];
    for (int i = 0; i < domainTypeCount; i++) {
        lookUpTable[i] = new double [4]();
        lookUpTable[i][0] = domain.getDomainR(i);
        lookUpTable[i][1] = domain.getDomainG(i);
        lookUpTable[i][2] = domain.getDomainB(i);
        lookUpTable[i][3] = 1;
        domainVisibility[i] = visibleAll;
    }


    cout << "Initialize vtk" << endl;

    // Create the renderer, render window and interactor
    /* vtkRenderer *renderer = vtkRenderer::New(); */

    /* If there are more than one file to visualize
     * then, instead of output the domain analytics data for each file
     * we output them into one single file call Domain_Analytics.txt*/
    if ( count > 1) {
        vector<string> header(domain.getDomainTypeCount()+1);
        header[0]="Index";
        for (int i = 0; i < domain.getDomainTypeCount(); i++) {
            header[i+1]=domain.getDomainTypeLabel(i); 
        }
        domainInfo.setFileName("Domain_Analytics.txt");
        domainInfo.initializeFile(header); 
    }

    for (int i = 0; i < count; i++) {

        cout <<setw(printWidth) << right << "Start to read data file " << fileName[i] <<endl;
        if( extension == "vti" )
        {
            vtiFileName.push_back(fileName[i]);
            imageName.push_back(fileName[i].substr(0,fileName[i].find_last_of("."))+".png");
        }
        else if ( extension == "dat" )
        {
            VectorDomainDataset dat;
            dat.setDatFileName(fileName[i]);
            dat.readDatFile();
            dat.getDimension(datDim);
            dat.setOutputDomainAnalysis(outputDomainAnalysis);

            dat.outputVTIFile(domain,1);

            if (count >1) {
                vector<double> value(domain.getDomainTypeCount());
                for (int j = 0; j < domain.getDomainTypeCount(); j++) {
                    value[j]=dat.getDomainPercent(j);
                }
                domainInfo.outputWithIndex(i*delta+start,value);
            }


            vtiFileName.push_back(dat.getVTIFileName());
            imageName.push_back(dat.getLongFileName()+".png");
        }
        else
        {
            cout << "Error! Not VTI or DAT!" << endl;
            exit(EXIT_FAILURE);
        }
    }


    focalX = datDim[0]/2;
    focalY = datDim[1]/2;
    focalZ = datDim[2]/2;
    positionX = datDim[0]*2;
    positionY = datDim[1]*2;
    positionZ = datDim[2]*2;
    upX = -1;
    upY = -1;
    upZ = 2;

    /* The LOOKUPTABLE, and some camera related values are 
     * set after reading the file because the they need some 
     * information of the data*/
    if (structRead.firstKeyExist("LOOKUPTABLE")) {
        choiceColor = structRead.getFirstLevel("lookuptable")[0];
        cout << setw(printWidth) << right << "Use the lookuptable " << choiceColor << endl; 
        if (structRead.secondKeyExist(choiceColor,"POINTADD")) {
            colorCount = structRead.getSecondLevel(choiceColor,"POINTADD").size();
            for (int i = 0; i < colorCount; i++) {
                istringstream(structRead.getSecondLevel(choiceColor,"POINTADD")[i]) >> Value >>R >> G >> B >> Alpha; 
                lookUpTable[Value][0] = R;
                lookUpTable[Value][1] = G;
                lookUpTable[Value][2] = B;
                lookUpTable[Value][3] = Alpha;
                cout << setw(printWidth) << right << "Add color pivot " << Value << " " << R << " " << G << " " << " " << B << " " << Alpha << endl;

            }
        }
    }else{
        cout << setw(printWidth) << right << "No lookuptable specified, use the default one" << endl;
    }

    if (structRead.firstKeyExist("CAMERA")) {
        cameraName = structRead.getFirstLevel("CAMERA")[0];
        cout << setw(printWidth) << right << "Choose to use the camera " << cameraName << endl;
        if (structRead.secondKeyExist(cameraName,"FOCAL")) {
            istringstream(structRead.getSecondLevel(cameraName,"FOCAL")[0]) >> focalX >> focalY >> focalZ;
            cout << setw(printWidth) << right << "Camera focal point set to " << focalX << " " << focalY << " " <<focalZ << endl;
        }else{
            cout << setw(printWidth) << right << "Use default value for camera focal point "<< focalX << " " << focalY << " " <<focalZ << endl;
        }

        if (structRead.secondKeyExist(cameraName,"POSITION")) {
            istringstream(structRead.getSecondLevel(cameraName,"POSITION")[0]) >> positionX >> positionY >> positionZ;
            cout << setw(printWidth) << right << "Camera position set to " << positionX << " " << positionY << " " <<positionZ << endl;
        }else{
            cout << setw(printWidth) << right << "Use default value for camera position "<< positionX << " " << positionY << " " <<positionZ << endl;
        }

        if (structRead.secondKeyExist(cameraName,"UP")) {
            istringstream(structRead.getSecondLevel(cameraName,"UP")[0]) >> upX >> upY >> upZ;
            cout << setw(printWidth) << right << "Camera up direction set to " << upX << " " << upY << " " <<upZ << endl;
        }else{
            cout << setw(printWidth) << right << "Use default value for camera up direction "<< upX << " " << upY << " " <<upZ << endl;
        }


    }else{
        cout << setw(printWidth) << right << "No camera specified, use the default one" << endl;
    }

    if (structRead.firstKeyExist("VISIBLE")) {
        istringstream temp(structRead.getFirstLevel("VISIBLE")[0]);
        while(temp >> number ) {
            domainVisibility[number] = true;
            cout << setw(printWidth) << right << "Visible domain label " << number << endl;
        }
    }

    if (structRead.firstKeyExist("NOTVISIBLE")) {
        istringstream temp(structRead.getFirstLevel("NOTVISIBLE")[0]);
        while(temp >> number ) {
            domainVisibility[number] = false;
            cout << setw(printWidth) << right << "Invisible domain label " << number << endl;
        }
    }





    /* Start to visualize for each file */
    for (int m = 0; m < count; m++) {

    VTK_CREATE(vtkRenderer,renderer);
    VTK_CREATE(vtkRenderWindow,renWin);

    VTK_CREATE(vtkRenderWindowInteractor,iren);
    VTK_CREATE(vtkInteractorStyleTrackballCamera,style);
    // Read the data
    VTK_CREATE(vtkXMLImageDataReader,reader);
    VTK_CREATE(vtkImageData,input);
    VTK_CREATE(vtkCamera,camera);


    VTK_CREATE(vtkColorTransferFunction,color);
    VTK_CREATE(vtkPiecewiseFunction,compositeOpacity);

    VTK_CREATE(vtkWindowToImageFilter,windowToImageFilter);
    VTK_CREATE(vtkPNGWriter,writer);



        vector<vtkActor *> actorDomain;
        vector<vtkDataSetMapper *> domainMapper;
        for (int i = 0; i < domainTypeCount; i++) {
            actorDomain.push_back(vtkActor::New());        
            domainMapper.push_back(vtkDataSetMapper::New());        
        }



        cout << setw(printWidth) << right << " Visualizing file " << vtiFileName[m] << endl;

        reader->SetFileName(vtiFileName[m].c_str());
        reader->Update();
        /* reader->Print(cout); */
        input->ShallowCopy(reader->GetOutput());



        /* Visualize each domain for one File */

        for (int i = 0; i < domainTypeCount; i++) {

            VTK_CREATE(vtkThreshold,domainThreshold);
            VTK_CREATE(vtkSmoothPolyDataFilter,domainSmooth);
            VTK_CREATE(vtkPolyDataNormals,normalGenerator);
            VTK_CREATE(vtkDataSetSurfaceFilter,domainSurface);


            domainThreshold->SetInputData(reader->GetOutput());

            /* cout << "Number of point array" << reader->GetNumberOfPointArrays() << reader->GetPointArrayStatus("domain")<< endl; */
            /* cout << "Number of cell array" << reader->GetNumberOfCellArrays() << endl; */
            domainThreshold->AllScalarsOff();
            domainThreshold->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,"domain");
            domainSmooth->SetInputConnection(domainSurface->GetOutputPort());
            domainSmooth->SetNumberOfIterations(30);
            domainSmooth->SetRelaxationFactor(0.1);
            domainSmooth->FeatureEdgeSmoothingOff();
            domainSmooth->BoundarySmoothingOn();

            domainMapper[i]->SetInputData( input );
            domainMapper[i]->SetInputConnection(normalGenerator->GetOutputPort());
            domainMapper[i]->ScalarVisibilityOff();

            /* cout << "Output domain of " << i << endl; */
            domainThreshold->ThresholdBetween(i-0.5,i+0.5);
            domainThreshold->Update();
            /* domainThreshold->Print(cout); */
            if(domainThreshold->GetOutput()->GetCells()->GetNumberOfCells() !=0){
                /* cout << "after threshold" << endl; */
                domainSurface->SetInputConnection(domainThreshold->GetOutputPort());
                domainSurface->Update();
                /* cout << "after surface" << endl; */
                domainSurface->SetInputConnection(domainThreshold->GetOutputPort());
                domainSmooth->Update();
                /* cout << "after smooth" << endl; */
                domainSurface->SetInputConnection(domainThreshold->GetOutputPort());
                normalGenerator->SetInputConnection(domainSmooth->GetOutputPort());
                normalGenerator->ComputePointNormalsOn();
                normalGenerator->ComputeCellNormalsOn();
                normalGenerator->UpdateWholeExtent();
                actorDomain[i]->SetMapper(domainMapper[i]);
                domainMapper[i]->Update();
                actorDomain[i]->GetProperty()->SetColor(lookUpTable[i][0],lookUpTable[i][1],lookUpTable[i][2]);
                actorDomain[i]->GetProperty()->SetOpacity(lookUpTable[i][3]);
                /* cout << "domain visiblity " << i << " " << domainVisibility[i] << endl; */
                actorDomain[i]->SetVisibility(domainVisibility[i]);
                actorDomain[i]->Modified();
                /* actorDomain[i]->Print(cout) ; */
                renderer->AddActor(actorDomain[i]); 

            }
        }



        renWin->AddRenderer(renderer);
        /* renderer->Print(cout) ; */


        renderer->SetBackground(1,1,1);

        /* renWin->SetSize(600,600); */
        /* iren->Initialize(); */

        /* cout << "viewWindow" << viewWindow << renderer->VisibleActorCount() << endl; */

        camera->SetFocalPoint(focalX,focalY,focalZ);
        camera->SetPosition(positionX,positionY,positionZ);
        camera->SetViewUp(upX,upY,upZ);
        /* camera->Update(); */
        renderer->SetActiveCamera(camera);
        if (viewWindow) {
            /* renderer->ResetCamera(); */
            cout << setw(printWidth) << right << "Opening render window for " << fileName[m] << endl;
            iren->SetRenderWindow(renWin);
            renWin->SetSize(windowX,windowY);
            renWin->Render();
            iren->SetDesiredUpdateRate(20);
            iren->SetInteractorStyle(style);
            iren->Start();
        }else{
            renWin->SetSize(windowX,windowY);
            cout << setw(printWidth) << right << "Rendering off screen " << imageName[m] << endl;
            renWin->SetOffScreenRendering(1);
            renWin->Render();
            windowToImageFilter->SetInput(renWin);
            windowToImageFilter->Update();
            writer->SetFileName(imageName[m].c_str());
            writer->SetInputConnection(windowToImageFilter->GetOutputPort());
            writer->Write();
        }
    }

//    compositeOpacity->Delete();
//    color->Delete();
//
//    /* delete domainMapper []; */
//    reader->Delete();
//    renderer->Delete();
//    renWin->Delete();
//    iren->Delete();
//    style->Delete();

    return 0;
}
