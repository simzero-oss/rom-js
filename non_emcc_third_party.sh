#!/bin/bash

PROC=6
ROOT=.
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

(cd $OPENFOAM_ROOT && ./Allwmake -j${PROC})

echo "#############################"
echo  BUILDING ITHACA-FV
echo "#############################"

ITHACAFV_ROOT=$BUILD_ROOT/ithaca-fv
APPLICATIONS=$ROOT/applications

source $ITHACAFV_ROOT/etc/bashrc

(cd $ITHACAFV_ROOT && ./Allwmake -j${PROC})
cd $APPLICATIONS/steady && wmake
