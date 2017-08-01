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

#include <FreeFormatOneLine.h>
#include <FreeFormatParser.h>
#include <VectorDomainDataset.h>
#include <FerroDomain.h>
#include <sstream>
#include <iostream>

#define VTI_FILETYPE 1
#define DAT_FILETYPE 2

#define VTK_CREATE(type, name) \
    vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace std;

void PrintUsage()
{
  cout << "Usage: " << endl;
  cout << endl;
  cout << "  FerroDomainVTK <options>" << endl;
  cout << endl;
  cout << "where options may include: " << endl;
  cout << endl;
  cout << "  -VTK <filename>" << endl;
  cout << "  -DAT <filename>" << endl;
  cout << "  -Clip" << endl;
  cout << "  -CompositeRamp <window> <level>" << endl;
  cout << "  -CompositeShadeRamp <window> <level>" << endl;
  cout << "  -FrameRate <rate>" << endl;
  cout << "  -DataReduction <factor>" << endl;
  cout << endl;
  cout << "You must use either the -DICOM option to specify the directory where" << endl;
  cout << "the data is located or the -VTI or -MHA option to specify the path of a .vti file." << endl;
  cout << endl;
  cout << "By default, the program assumes that the file has independent components," << endl;
  cout << "use -DependentComponents to specify that the file has dependent components." << endl;
  cout << endl;
  cout << "Use the -Clip option to display a cube widget for clipping the volume." << endl;
  cout << "Use the -FrameRate option with a desired frame rate (in frames per second)" << endl;
  cout << "which will control the interactive rendering rate." << endl;
  cout << "Use the -DataReduction option with a reduction factor (greater than zero and" << endl;
  cout << "less than one) to reduce the data before rendering." << endl;
  cout << "Use one of the remaining options to specify the blend function" << endl;
  cout << "and transfer functions. The -MIP option utilizes a maximum intensity" << endl;
  cout << "projection method, while the others utilize compositing. The" << endl;
  cout << "-CompositeRamp option is unshaded compositing, while the other" << endl;
  cout << "compositing options employ shading." << endl;
  cout << endl;
  cout << "Note: MIP, CompositeRamp, CompositeShadeRamp, CT_Skin, CT_Bone," << endl;
  cout << "and CT_Muscle are appropriate for DICOM data. MIP, CompositeRamp," << endl;
  cout << "and RGB_Composite are appropriate for RGB data." << endl;
  cout << endl;
  cout << "Example: FixedPointVolumeRayCastMapperCT -DICOM CTNeck -MIP 4096 1024" << endl;
  cout << endl;
}

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

  int count = 1;
  char *dirname = NULL;
  double opacityWindow = 4096;
  double opacityLevel = 2048;
  int blendType = 0;
  int clip = 0;
  double reductionFactor = 1.0;
  double frameRate = 10.0;
  char *fileName=0;
  int fileType=0;
  char **firstLevel[300],**secondLevel[300];

  bool independentComponents=true;


  FreeFormatParser b;
  b.setFilename("struct.in");
  b.parse();
  std::cout << "The first Level"<<b.getFirstLevel("Plane") << endl;
  string c=b.getSecondLevel("COLUMN1","SHAPE"); 
  std::cout << "The second level" << c<< endl;
  int x;
  istringstream(c) >> x;
  cout << "x="<<x<<endl;
  double m,n,p;
  istringstream(b.getFirstLevel("simdim")) >> m >> n >> p;
  cout << "mnp="<<m<<n<<p<<endl;

  FerroDomain ferroDomain;
  ferroDomain.setStandardValue(0.1);
  ferroDomain.setStandardAngle(180);
  

  while ( count < argc )
  {
    if ( !strcmp( argv[count], "?" ) )
    {
      PrintUsage();
      exit(EXIT_SUCCESS);
    }
    else if ( !strcmp( argv[count], "-VTI" ) )
    {
      fileName = new char[strlen(argv[count+1])+1];
      fileType = VTI_FILETYPE;
      sprintf( fileName, "%s", argv[count+1] );
      count += 2;
    }
    else if ( !strcmp( argv[count], "-DAT" ) )
    {
      fileName = new char[strlen(argv[count+1])+1];
      fileType = DAT_FILETYPE;
      sprintf( fileName, "%s", argv[count+1] );
      count += 2;
    }
    else if ( !strcmp( argv[count], "-Clip") )
    {
      clip = 1;
      count++;
    }
    else if ( !strcmp( argv[count], "-CompositeRamp" ) )
    {
      opacityWindow = atof( argv[count+1] );
      opacityLevel  = atof( argv[count+2] );
      blendType = 1;
      count += 3;
    }
    else if ( !strcmp( argv[count], "-CompositeShadeRamp" ) )
    {
      opacityWindow = atof( argv[count+1] );
      opacityLevel  = atof( argv[count+2] );
      blendType = 2;
      count += 3;
    }
    else if ( !strcmp( argv[count], "-RGB_Composite" ) )
    {
      blendType = 6;
      count += 1;
    }
    else if ( !strcmp( argv[count], "-FrameRate") )
    {
      frameRate = atof( argv[count+1] );
      if ( frameRate < 0.01 || frameRate > 60.0 )
      {
        cout << "Invalid frame rate - use a number between 0.01 and 60.0" << endl;
        cout << "Using default frame rate of 10 frames per second." << endl;
        frameRate = 10.0;
      }
      count += 2;
    }
    else if ( !strcmp( argv[count], "-ReductionFactor") )
    {
      reductionFactor = atof( argv[count+1] );
      if ( reductionFactor <= 0.0 || reductionFactor >= 1.0 )
      {
        cout << "Invalid reduction factor - use a number between 0 and 1 (exclusive)" << endl;
        cout << "Using the default of no reduction." << endl;
        reductionFactor = 1.0;
      }
      count += 2;
    }
     else if ( !strcmp( argv[count], "-DependentComponents") )
     {
      independentComponents=false;
      count += 1;
     }
    else
    {
      cout << "Unrecognized option: " << argv[count] << endl;
      cout << endl;
      PrintUsage();
      exit(EXIT_FAILURE);
    }
  }

  if ( !dirname && !fileName)
  {
    cout << "Error: you must specify a .vtk or .dat data file!" << endl;
    cout << endl;
    PrintUsage();
    exit(EXIT_FAILURE);
  }
  

  // Create the renderer, render window and interactor
  /* vtkRenderer *renderer = vtkRenderer::New(); */
  VTK_CREATE(vtkRenderer,renderer);
  VTK_CREATE(vtkRenderWindow,renWin);
  renWin->AddRenderer(renderer);

  // Connect it all. Note that funny arithematic on the
  // SetDesiredUpdateRate - the vtkRenderWindow divides it
  // allocated time across all renderers, and the renderer
  // divides it time across all props. If clip is
  // t[M#Arue then there are two props
  VTK_CREATE(vtkRenderWindowInteractor,iren);
  iren->SetRenderWindow(renWin);
  VTK_CREATE(vtkInteractorStyleTrackballCamera,style);
  iren->SetInteractorStyle(style);
  renWin->Render();
 // Read the data
  VTK_CREATE(vtkXMLImageDataReader,reader);
  VTK_CREATE(vtkImageData,input);

  VTK_CREATE(vtkThreshold,domainThreshold);
  VTK_CREATE(vtkDataSetMapper,domainMapper);
  VTK_CREATE(vtkSmoothPolyDataFilter,domainSmooth);
  VTK_CREATE(vtkPolyDataNormals,normalGenerator);
  VTK_CREATE(vtkDataSetSurfaceFilter,domainSurface);





  cout <<"STartTo read data file"<<endl;
  if( fileType == VTI_FILETYPE )
  {
    reader->SetFileName(fileName);
    reader->Update();
  }
  else if ( fileType == DAT_FILETYPE )
  {


  VectorDomainDataset dat;
  int datDim[4];
  dat.setDatFileName(fileName);
  dat.readDatFile();
  
  dat.outputVTIFile(ferroDomain,1);
  dat.getDimension(datDim);
  std::string vtiFile = dat.getVTIFileName(); 
    reader->SetFileName(vtiFile.c_str());
    reader->Update();
    std::cout << "dat readder successful"<<std::endl;
  }
  else
  {
    cout << "Error! Not VTK or DAT!" << endl;
    exit(EXIT_FAILURE);
  }

  input->ShallowCopy(reader->GetOutput());

  // Create our volume and mapper
  cout << reader->GetPointArrayStatus(reader->GetPointArrayName(1)) << endl;;
  domainMapper->SetInputData( input );


  // Set the sample distance on the ray to be 1/2 the average spacing

//  VTK_CREATE(vtkCallbackCommand,keypressCallback);
//  keypressCallback->SetCallback(KeypressCallbackFunction);
//  keypressCallback->SetClientData(mapper);
//  iren->AddObserver(vtkCommand::KeyPressEvent,keypressCallback);


  // Create our transfer function
  /* vtkColorTransferFunction *colorFun = vtkColorTransferFunction::New(); */
  VTK_CREATE(vtkColorTransferFunction,color);
  /* vtkPiecewiseFunction *opacityFun = vtkPiecewiseFunction::New(); */
  VTK_CREATE(vtkPiecewiseFunction,compositeOpacity);

  // Create the property and attach the transfer functions
  /* vtkVolumeProperty *property = vtkVolumeProperty::New(); */


std:vector<vtkActor *> actorDomain;
    for (int i = 0; i < 27; i++) {
        actorDomain.push_back(vtkActor::New());        
    }




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
  renWin->Render();
  iren->Start();

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
