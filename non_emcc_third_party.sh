#!/bin/bash

PROC=6
ROOT=/build
BUILD_ROOT=$ROOT/third_party

echo "#############################"
echo  INITIALIZING THIRD PARTY CODE
echo "#############################"

git submodule update --init --recursive

echo "#############################"
echo  BUILDING OpenFOAM
echo "#############################"

OPENFOAM_ROOT=$BUILD_ROOT/openfoam
OPENFOAM_BUILD=$OPENFOAM_ROOT/openfoam-build

source $OPENFOAM_ROOT/etc/bashrc

cd $OPENFOAM_ROOT

pwd

./Allwmake -j${PROC}

echo "#############################"
echo  BUILDING ITHACA-FV
echo "#############################"

ITHACAFV_ROOT=$BUILD_ROOT/ITHACA-FV
APPLICATIONS=$ROOT/applications

cd $ITHACAFV_ROOT

./Allwmake -j${PROC}
cd $APPLICATIONS/steady && wmake
