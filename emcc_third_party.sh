#!/bin/bash

PROC=6
ROOT=/build
BUILD_ROOT=$ROOT/third_party

echo "#############################"
echo  BUILDING VTK
echo "#############################"

VTK_ROOT=$BUILD_ROOT/vtk
VTK_BUILD=$BUILD_ROOT/vtk/vtk-wasm
VTK_VERSION=v9.1.0
VTK_LIB_VERSION=9.1

if [ ! -d $VTK_BUILD  ];then
  mkdir -p $VTK_BUILD
fi

cd $VTK_BUILD

cmake \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
  -DVTK_BUILD_TESTING=OFF \
  -DVTK_BUILD_DOCUMENTATION=NO \
  -DVTK_GROUP_ENABLE_Imaging=DONT_WANT \
  -DVTK_GROUP_ENABLE_MPI=DONT_WANT \
  -DVTK_GROUP_ENABLE_Qt=DONT_WANT \
  -DVTK_GROUP_ENABLE_Rendering=WANT \
  -DVTK_GROUP_ENABLE_StandAlone=WANT \
  -DVTK_GROUP_ENABLE_Views=DONT_WANT \
  -DVTK_GROUP_ENABLE_Web=DONT_WANT \
  -DBUILD_SHARED_LIBS:BOOL=OFF \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DVTK_ENABLE_LOGGING:BOOL=OFF \
  -DVTK_ENABLE_WRAPPING:BOOL=OFF \
  -DVTK_LEGACY_REMOVE:BOOL=ON \
  -DVTK_OPENGL_USE_GLES:BOOL=ON \
  -DVTK_USE_SDL2:BOOL=ON \
  -DVTK_NO_PLATFORM_SOCKETS:BOOL=ON \
  -DVTK_MODULE_ENABLE_VTK_hdf5:STRING=NO \
  -DVTK_MODULE_ENABLE_VTK_libproj=NO \
  -DVTK_MODULE_ENABLE_VTK_RenderingContext2D=DONT_WANT \
  -DVTK_MODULE_ENABLE_VTK_RenderingContextOpenGL2=DONT_WANT \
  $VTK_ROOT

cmake --build $VTK_BUILD --parallel $PROC

echo "#############################"
echo  BUILDING SPLINTER
echo "#############################"

ITHACA_THIRD_PARTY=ithaca-fv/src/ITHACA_THIRD_PARTY/splinter
SPLINTER_ROOT=$BUILD_ROOT/splinter
SPLINTER_SOURCE=$SPLINTER_ROOT/src
SPLINTER_INCLUDE=$SPLINTER_ROOT/include
SPLINTER_BUILD=$SPLINTER_ROOT/splinter-wasm
SPLINTER_RBF_FILE=$SPLINTER_SOURCE/rbfspline.cpp
SPLINTER_RBF_INCLUDE=$SPLINTER_INCLUDE/rbfspline.h
SPLINTER_SPLINE_INCLUDE=$SPLINTER_INCLUDE/spline.h

if [ ! -f $SPLINTER_RBF_FILE ];then
  cd $SPLINTER_SOURCE
  ln -s ../../$ITHACA_THIRD_PARTY/src/rbfspline.C rbfspline.cpp
fi

if [ ! -f $SPLINTER_RBF_INCLUDE ];then
  cd $SPLINTER_INCLUDE
  ln -s ../../$ITHACA_THIRD_PARTY/include/rbfspline.h rbfspline.h
fi

if [ ! -f $SPLINTER_SPLINE_INCLUDE ];then
  cd $SPLINTER_INCLUDE
  ln -s ../../$ITHACA_THIRD_PARTY/include/spline.h spline.h
fi

if [ ! -d $SPLINTER_BUILD  ];then
  cd $SPLINTER_ROOT
  mkdir -p $SPLINTER_BUILD
fi

cd $SPLINTER_BUILD

patch --forward -d $SPLINTER_ROOT < $BUILD_ROOT/splinter.patch
patch --forward -d $BUILD_ROOT/$ITHACA_THIRD_PARTY/include < $BUILD_ROOT/ithaca-fv.patch

emcmake cmake .. \
  -G Ninja -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
  -DCMAKE_BUILD_TYPE=Release \
  -DARCH=wasm64 \
  -Wno-dev

cmake --build .
