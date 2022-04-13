#!/bin/sh

echo "#############################"
echo  BUILDING rom.js
echo "#############################"

ROOT=/build
BUILD_ROOT=$ROOT/third_party

VTK_ROOT=$BUILD_ROOT/vtk
VTK_BUILD=$BUILD_ROOT/vtk/vtk-wasm
VTK_VERSION=v9.1.0
VTK_LIB_VERSION=9.1
EIGEN_ROOT=$BUILD_ROOT/eigen
EIGEN_VERSION=3.4.0
SPLINTER_ROOT=$BUILD_ROOT/splinter
SPLINTER_SOURCE=$SPLINTER_ROOT/src
SPLINTER_INCLUDE=$SPLINTER_ROOT/include
SPLINTER_BUILD=$SPLINTER_ROOT/splinter-wasm
ITHACA_ROMPROBLEMS=$BUILD_ROOT/ithaca-fv/src/ITHACA_ROMPROBLEMS

if [ ! -d ./build  ];then
  mkdir -p ./build
fi

# TODO: develop CMakeLists.txt
emcc -D NO_OSQP \
  -I $VTK_BUILD/IO/XML \
  -I $VTK_BUILD/IO/XMLParser \
  -I $VTK_BUILD/Common/ExecutionModel \
  -I $VTK_BUILD/Common/Core \
  -I $VTK_BUILD/Common/Misc \
  -I $VTK_BUILD/Utilities/KWIML \
  -I $VTK_BUILD/Filters/Core \
  -I $VTK_BUILD/Filters/Geometry \
  -I $VTK_BUILD/Common/DataModel \
  -I $VTK_ROOT/IO/Core \
  -I $VTK_ROOT/IO/XML \
  -I $VTK_ROOT/IO/XMLParse \
  -I $VTK_ROOT/Common/ExecutionModel \
  -I $VTK_ROOT/Common/DataModel \
  -I $VTK_ROOT/Common/Core \
  -I $VTK_ROOT/Common/Math \
  -I $VTK_ROOT/Common/Misc \
  -I $VTK_ROOT/Common/Sytem \
  -I $VTK_ROOT/Utilities/KWIML \
  -I $VTK_ROOT/Filters/Core \
  -I $VTK_ROOT/Filters/Geometry \
  -I $VTK_ROOT/Common/DataModel \
  -I $ITHACA_ROMPROBLEMS/NonLinearSolvers \
  -I $SPLINTER_INCLUDE \
  -I $EIGEN_ROOT \
  -L $VTK_BUILD/lib \
  -lvtkdoubleconversion-$VTK_LIB_VERSION \
  -lvtkFiltersCore-$VTK_LIB_VERSION \
  -lvtkFiltersGeometry-$VTK_LIB_VERSION \
  -lvtkCommonCore-$VTK_LIB_VERSION \
  -lvtkCommonDataModel-$VTK_LIB_VERSION \
  -lvtkCommonExecutionModel-$VTK_LIB_VERSION \
  -lvtkCommonMath-$VTK_LIB_VERSION \
  -lvtkCommonMisc-$VTK_LIB_VERSION \
  -lvtkCommonSystem-$VTK_LIB_VERSION \
  -lvtkCommonTransforms-$VTK_LIB_VERSION \
  -lvtkexpat-$VTK_LIB_VERSION \
  -lvtkpugixml-$VTK_LIB_VERSION \
  -lvtkIOCore-$VTK_LIB_VERSION \
  -lvtkIOXMLParser-$VTK_LIB_VERSION \
  -lvtkIOXML-$VTK_LIB_VERSION \
  -lvtksys-$VTK_LIB_VERSION \
  -lvtkzlib-$VTK_LIB_VERSION \
  -lvtklzma-$VTK_LIB_VERSION \
  -lvtklz4-$VTK_LIB_VERSION \
  -L $SPLINTER_BUILD \
  -lsplinter-3-0 \
  -Isrc \
  -s EMULATE_FUNCTION_POINTER_CASTS=0 \
  -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
  -s DISABLE_EXCEPTION_CATCHING=0 \
  -s ASSERTIONS=0 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s MODULARIZE=1 \
  -O3 \
  --bind \
  -o ./build/rom.js \
  ./src/embind.cc
