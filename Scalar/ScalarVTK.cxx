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
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkGPUVolumeRayCastMapper.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkXMLImageDataReader.h"
#include "vtkImageData.h"
#include "vtkCallbackCommand.h"
#include "vtkPointData.h"
#include "vtkInteractorStyleTrackballCamera.h"

#include "vtkScalarBarWidget.h"
#include "vtkScalarBarActor.h"
#include "vtkCamera.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
 

#include <FreeFormatOneLine.h>
#include <FreeFormatParser.h>
#include <Dataset.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#define VTI_FILETYPE 1
#define DAT_FILETYPE 2

#define VTK_CREATE(type, name) \
    vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace std;



void KeypressCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData){
    /* VTK_CREATE(vtkSmartVolumeMapper,mapper); */
    vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);

    vtkSmartVolumeMapper* map=reinterpret_cast<vtkSmartVolumeMapper*>(clientData);

    int number=(int)*iren->GetKeySym()-49;
    int colNum=map->GetInput()->GetPointData()->GetNumberOfArrays();
    cout << "Pressed:" << number<<colNum<<endl;
    if (number<=colNum-1 && number >=0) {
        map->SelectScalarArray(number);
        map->Update();
    }

}

int main(int argc, char *argv[])
{
    // Parse the parameters

    char *dirname = NULL;
    double opacityWindow = 4096;
    double opacityLevel = 2048;
    int blendType = 0;
    int clip = 0;
    vector<string> fileName;
    vector<string> vtiFileName;
    vector<string> imageName;

    bool fileFormatSTD=true;
    bool viewWindow = true;
    bool outputScalarAnalysis = false;
    double R,G,B,Alpha,Value;
    string shortFileName="",midFileName="",extension="";

    int start=0,delta=0,end=0,count=1;
    double focalX,focalY,focalZ;
    double positionX,positionY,positionZ;
    double upX,upY,upZ;
    string cameraName;
    int datDim[4];
    int printWidth=80;

    double reductionFactor = 1.0;
    double frameRate = 10.0;
    int fileType=0;
    char **firstLevel[300],**secondLevel[300];

    bool independentComponents=true;
    vector<double*> lookUpTable;
    double colorHold[5];
    bool controlFileExist=false;
    bool showLegend=true;
    int colorCount=0;
    string choiceColor;

    ostringstream stringstream;

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



    /* cout << argc << "argc" << endl; */
    /* If no filename is passed using arguement, 
     * then read FILENAME, EXTENSION, START, DELTA, END
     * from the struct.in file*/

    if (argc==1) {
        viewWindow=false;
        outputScalarAnalysis=false;

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
        outputScalarAnalysis=true;
        extension=fileName[0].substr(fileName[0].find_last_of(".")+1);
        cout << setw(printWidth) << "Visualize the file " << fileName[0] << endl;
        cout << setw(printWidth) << "File extension is " << extension << endl;
    }

    if (structRead.firstKeyExist("SHOWLEGEND")) {
        istringstream(structRead.getFirstLevel("SHOWLEGEND")[0]) >> boolalpha >>  showLegend;
        cout << setw(printWidth) << right << "Choose to use show the scalar legend bar " << boolalpha << showLegend << endl;
    }else{
        cout << setw(printWidth) << right << "Default is showing the scalar legend bar " << showLegend << endl;
    }




    cout << "Finished reading the visual.in file." << endl;


    cout << "Initialize vtk." << endl;
    // Create the renderer, render window and interactor
    /* vtkRenderer *renderer = vtkRenderer::New(); */


    if (count > 1) {
        // a place holder for the future scalar data analysis
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
            Dataset dat;
            dat.setDatFileName(fileName[i]);
            dat.readDatFile();
            dat.getDimension(datDim);

            dat.outputVTIFile();

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
                colorHold[0] = Value;
                colorHold[1] = R;
                colorHold[2] = G;
                colorHold[3] = B;
                colorHold[4] = Alpha;
                lookUpTable.push_back(colorHold);
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

    for (int m = 0; m < count; m++) {

        VTK_CREATE(vtkRenderer,renderer);
        VTK_CREATE(vtkRenderWindow,renWin);

        // Connect it all. Note that funny arithematic on the
        // SetDesiredUpdateRate - the vtkRenderWindow divides it
        // allocated time across all renderers, and the renderer
        // divides it time across all props. If clip is
        // true then there are two props
        VTK_CREATE(vtkRenderWindowInteractor,iren);
        VTK_CREATE(vtkInteractorStyleTrackballCamera,style);
        // Read the data
        VTK_CREATE(vtkXMLImageDataReader,reader);
        VTK_CREATE(vtkImageData,input);

        VTK_CREATE(vtkVolume,volume);
        VTK_CREATE(vtkSmartVolumeMapper,mapper);
        VTK_CREATE(vtkColorTransferFunction,color);
        VTK_CREATE(vtkPiecewiseFunction,compositeOpacity);
        VTK_CREATE(vtkVolumeProperty,property);

        VTK_CREATE(vtkScalarBarWidget,scalarLegendWidget);
        VTK_CREATE(vtkScalarBarActor,scaleBarActor);
        VTK_CREATE(vtkCamera,camera);
        VTK_CREATE(vtkWindowToImageFilter,windowToImageFilter);
        VTK_CREATE(vtkPNGWriter,writer);

    VTK_CREATE(vtkCallbackCommand,keypressCallback);
    keypressCallback->SetCallback(KeypressCallbackFunction);
    keypressCallback->SetClientData(mapper);


        



        cout << setw(printWidth) << right << "Visualizing file " << vtiFileName[m] << endl;
        reader -> SetFileName(vtiFileName[m].c_str());
        reader->Update();
//        reader->GetOutput()->GetPointData()->GetScalars()->GetRange(scalarRange);
//        if (colorCount == 0) {
//        colorHold[0] = scalarMin;
//        colorHold[1] = 0;
//        colorHold[2] = 0;
//        colorHold[3] = 1.0;
//        colorHold[4] = 1.0;
//        lookUpTable.push_back(colorHold);
//        colorHold[0] = (scalarMin+scalarMax)/2.0;
//        colorHold[1] = 0;
//        colorHold[2] = 1.0;
//        colorHold[3] = 0.0;
//        colorHold[4] = 1.0;
//        lookUpTable.push_back(colorHold);
//        colorHold[0] = scalarMax;
//        colorHold[1] = 1.0;
//        colorHold[2] = 0.0;
//        colorHold[3] = 0.0;
//        colorHold[4] = 1.0;
//        lookUpTable.push_back(colorHold);
//        }


        /* input->ShallowCopy(reader->GetOutput()); */

        // Create our volume and mapper
        /* cout << reader->GetPointArrayStatus(reader->GetPointArrayName(1)) << endl;; */
        mapper->SetInputData( reader->GetOutput() );
        mapper->SetBlendModeToComposite();


        // Set the sample distance on the ray to be 1/2 the average spacing



        // Create our transfer function
        /* vtkColorTransferFunction *colorFun = vtkColorTransferFunction::New(); */
        /* vtkPiecewiseFunction *opacityFun = vtkPiecewiseFunction::New(); */

        // Create the property and attach the transfer functions
        /* vtkVolumeProperty *property = vtkVolumeProperty::New(); */
        /* property->ShadeOff(); */

        if (colorCount>0) {
        for (int i = 0; i < lookUpTable.size(); i++) {
            compositeOpacity->AddPoint(lookUpTable[i][0],lookUpTable[i][4]);
            color->AddRGBPoint(lookUpTable[i][0],lookUpTable[i][1],lookUpTable[i][2],lookUpTable[i][3]);
        }
        property->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
        property->SetScalarOpacity(compositeOpacity); // composite first.
        property->SetColor(color);
        // connect up the volume to the property and the mapper
        volume->SetProperty( property );
        }
        volume->SetMapper( mapper );

        mapper->SetScalarModeToUsePointFieldData();
        mapper->SelectScalarArray(5);
        mapper->Update();
        volume->Update();
        renderer->AddViewProp( volume );

        mapper->SetRequestedRenderModeToRayCast();
        renderer->SetActiveCamera(camera);

        renWin->AddRenderer(renderer);
        /* renWin->SetSize(600,600); */
        /* iren->Initialize(); */
        scalarLegendWidget->SetScalarBarActor(scaleBarActor);
        scalarLegendWidget->ResizableOn();
            if (showLegend) {
                scalarLegendWidget->On();
            }else{
                scalarLegendWidget->Off();
            }


        if (viewWindow) {
            cout << setw(printWidth) << right << "Opening render window for " << fileName[m] << endl;
            scalarLegendWidget->SetInteractor(iren);

            iren->AddObserver(vtkCommand::KeyPressEvent,keypressCallback);
            iren->SetRenderWindow(renWin);

            renWin->SetSize(600,600);
            renWin->Render();
            iren->SetDesiredUpdateRate(20);
            iren->SetInteractorStyle(style);
            iren->Start();
        }else{
            cout << setw(printWidth) << right << "Rendering off screen "<< imageName[m] << endl;
            renWin->SetOffScreenRendering(1);
            renWin->Render();
            windowToImageFilter->SetInput(renWin);
            windowToImageFilter->Update();
            writer->SetFileName(imageName[m].c_str());
            writer->SetInputConnection(windowToImageFilter->GetOutputPort());
            writer->Write();

        }

//    color->Delete();
//    property->Delete();
//
//    volume->Delete();
//    mapper->Delete();
//    reader->Delete();
//    renderer->Delete();
//    renWin->Delete();
//    iren->Delete();
//    style->Delete();
//    color->Delete();
//    compositeOpacity->Delete();


    }
    return 0;
}
