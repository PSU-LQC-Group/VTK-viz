cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set (VTK_DIR
   /gpfs/group/lqc3/default/VTK-static/lib/cmake/vtk-8.1
   )

set (CMAKE_CXX_COMPILER
    /opt/rh/devtoolset-4/root/usr/bin/c++
  )

set (CMAKE_BUILD_TYPE
    Release
    )



project (VolumeRendering)
find_package(VTK REQUIRED)
vtk_module_config(VTK
  vtkCommonCore
  vtkIOImage
  vtkIOXML
  vtkInteractionWidgets
  vtkRenderingVolume${VTK_RENDERING_BACKEND}
)
set (SRCS
    FreeFormatOneLine.cxx
    FreeFormatParser.cxx
    SplittableString.cxx
    ScreenPrint.cxx
    )

set (HDRS
    FreeFormatOneLine.h
    FreeFormatParser.h
    SplittableString.h
    ScreenPrint.h
    )

set(RENDERING_EXAMPLES_SRCS
    ScalarVTK
    #/* FixedPointVolumeRayCastMapperCT */
    #/* GPURenderDemo */
)

set (EXECUTABLE_OUTPUT_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/../bin/
    )

include(${VTK_USE_FILE})

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../IO/
    )



foreach(name ${RENDERING_EXAMPLES_SRCS})
    add_executable(${name} MACOSX_BUNDLE ${name}.cxx)# ${SRCS} ${HDRS})
  target_link_libraries(${name} ${VTK_LIBRARIES})
  target_link_libraries(${name} ${CMAKE_SOURCE_DIR}/../IO/libGeneralIO.a)
endforeach()



