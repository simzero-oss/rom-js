// Copyright (c) 2022 Carlos Peña-Monferrer. All rights reserved.
// This work is licensed under the terms of the GNU LGPL v3.0 license.
// For a copy, see <https://opensource.org/licenses/LGPL-3.0>.

#include <emscripten/bind.h>
#include <iostream>
#include "reducedSteady.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Module)
{
    emscripten::register_vector<double>("Vector");
    emscripten::register_vector<std::vector<double>>("VectorVector");

    class_<ReducedSteady>("reducedSteady")
        .constructor<int, int>()
        .constructor<const ReducedSteady &>()
        .function("Nphi_u", &ReducedSteady::Nphi_u)
        .function("Nphi_p", &ReducedSteady::Nphi_p)
        .function("Nphi_nut", &ReducedSteady::Nphi_nut)
        .function("N_BC", &ReducedSteady::setBC)
        .function("addMatrices", &ReducedSteady::addMatrices)
        .function("addCMatrix", &ReducedSteady::addCMatrix)
        .function("addCt1Matrix", &ReducedSteady::addCt1Matrix)
        .function("addCt2Matrix", &ReducedSteady::addCt2Matrix)
        .function("addModes", &ReducedSteady::addModes)
        .function("setRBF", &ReducedSteady::setRBF)
        .function("nu", &ReducedSteady::nu)
        .function("preprocess", &ReducedSteady::preprocess)
        .function("solveOnline", &ReducedSteady::solveOnline)
        .function("reconstruct", &ReducedSteady::reconstruct)
        .function("exportUnstructuredGrid", &ReducedSteady::exportUnstructuredGrid)
        .function("unstructuredGridToPolyData", &ReducedSteady::unstructuredGridToPolyData)
        .function("readUnstructuredGrid", &ReducedSteady::readUnstructuredGrid)
	;
}
