// Copyright (c) 2022 Carlos Peña-Monferrer. All rights reserved.
// Adapted from ITHACA-FV ReducedSteadyNS and ReducedSteadyNSTurb classes
// on https://github.com/mathLab/ITHACA-FV with the following copyright
// and license:
/*---------------------------------------------------------------------------*\
     ██╗████████╗██╗  ██╗ █████╗  ██████╗ █████╗       ███████╗██╗   ██╗
     ██║╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗      ██╔════╝██║   ██║
     ██║   ██║   ███████║███████║██║     ███████║█████╗█████╗  ██║   ██║
     ██║   ██║   ██╔══██║██╔══██║██║     ██╔══██║╚════╝██╔══╝  ╚██╗ ██╔╝
     ██║   ██║   ██║  ██║██║  ██║╚██████╗██║  ██║      ██║      ╚████╔╝
     ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝      ╚═╝       ╚═══╝
 * In real Time Highly Advanced Computational Applications for Finite Volumes
 * Copyright (C) 2017 by the ITHACA-FV authors
-------------------------------------------------------------------------------
License
    This file is part of ITHACA-FV
    ITHACA-FV is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    ITHACA-FV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with ITHACA-FV. If not, see <http://www.gnu.org/licenses/>.
Description
    A reduced problem for the stationary turbulent NS equations
SourceFiles
    reducedSteady.h
\*---------------------------------------------------------------------------*/

#ifndef REDUCEDSTEADY_H
#define REDUCEDSTEADY_H

#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <list>

#include <vector>
#include <Eigen/Dense>
#include "newton_argument.H"
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

#include <bspline.h>
#include <bsplinebuilder.h>
#include <rbfspline.h>
#include <spline.h>
#include <datatable.h>

#include <vtkGeometryFilter.h>
#include <vtkNew.h>
#include <vtkDoubleArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkSmartPointer.h>
#include <vtkCellDataToPointData.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

// TODO:To deprecate
#define MAXBUFSIZE  ((int) 1e6)

using namespace std;
using namespace Eigen;

struct NewtonSteady: public newton_argument<double>
{
  NewtonSteady() {}
  NewtonSteady(int Nx, int Ny) : newton_argument<double>(Nx,Ny),
    Nphi_u(),
    Nphi_p(),
    Nphi_nut()
  {
  }

  int Nphi_u;
  int Nphi_p;
  int Nphi_nut;
  int N_BC;
  double nu;

  MatrixXd P;
  MatrixXd M;
  MatrixXd K;
  MatrixXd B;
  MatrixXd modes;
  MatrixXd solution;
  VectorXd BC;
  VectorXd g_nut;

  vector<MatrixXd> slicedC;
  vector<MatrixXd> slicedCt;

  vector<MatrixXd> CList;
  vector<int> CListMap;
  vector<MatrixXd> Ct1List;
  vector<int> Ct1ListMap;
  vector<MatrixXd> Ct2List;
  vector<int> Ct2ListMap;

  vector<SPLINTER::RBFSpline*> rbfSplines;

  vtkSmartPointer<vtkUnstructuredGrid> grid =
    vtkSmartPointer<vtkUnstructuredGrid>::New();

  int operator()(const VectorXd &x, VectorXd &fvec) const
  {
    VectorXd a_tmp(Nphi_u);
    VectorXd b_tmp(Nphi_p);
    a_tmp = x.head(Nphi_u);
    b_tmp = x.tail(Nphi_p);
    MatrixXd cc(1, 1);
    VectorXd M1 = B * a_tmp * nu;
    VectorXd M2 = K * b_tmp;
    VectorXd M3 = P * a_tmp;

    for (int i = 0; i < Nphi_u; i++)
    {
      auto it = find(CListMap.begin(), CListMap.end(), i);
      auto index = distance(CListMap.begin(), it);

      if (g_nut.rows() > 0)
      {
        cc = a_tmp.transpose() * slicedC[i] * a_tmp - g_nut.transpose() *
             slicedCt[i] * a_tmp;
      }
      else
      {
        cc = a_tmp.transpose() * slicedC[i] * a_tmp;
      }

      fvec(i) = M1(i) - cc(0, 0) - M2(i);
    }

    for (int j = 0; j < Nphi_p; j++)
    {
      int k = j + Nphi_u;
      fvec(k) = M3(j);
    }

    for (int j = 0; j < N_BC; j++)
    {
      fvec(j) = x(j) - BC(j);
    }

    return 0;
  }
 
  int df(const VectorXd &x, MatrixXd &fjac) const
  {
    NumericalDiff<NewtonSteady> numDiff(*this);
    numDiff.df(x, fjac);

    return 0;
  }
};

class ReducedSteady {

public:
  ReducedSteady(int x, int y)
    : x(x), y(y)
  {
    NewtonObject = NewtonSteady(x, y);
  }

  void addCMatrix(string const& M, int index) {

    NewtonObject.CList.push_back(readMatrix(M));
    NewtonObject.CListMap.push_back(index);
  }

  void addCt1Matrix(string const& M, int index) {
    NewtonObject.Ct1List.push_back(readMatrix(M));
    NewtonObject.Ct1ListMap.push_back(index);
  }

  void addCt2Matrix(string const& M, int index) {
    NewtonObject.Ct2List.push_back(readMatrix(M));
    NewtonObject.Ct2ListMap.push_back(index);
  }

  void addModes(string const& M) {
    NewtonObject.modes = readMatrix(M);
  }

  void setBC(int x_) {
    NewtonObject.BC.resize(x_);
    NewtonObject.N_BC = x_;
  }

  void preprocess() {
    for (int i = 0; i < NewtonObject.Nphi_u; i++)
    {
      auto it = find(NewtonObject.CListMap.begin(),
                          NewtonObject.CListMap.end(), i);
      auto index = distance(NewtonObject.CListMap.begin(), it);
      NewtonObject.slicedC.push_back(NewtonObject.CList[index]);
    }
    if (NewtonObject.g_nut.rows() > 0)
    {
      for (int i = 0; i < NewtonObject.Nphi_u; i++)
      {
        auto it = find(NewtonObject.Ct1ListMap.begin(),
                            NewtonObject.Ct1ListMap.end(), i);
        auto index = distance(NewtonObject.Ct1ListMap.begin(), it);
	MatrixXd M1 = NewtonObject.Ct1List[index];
	MatrixXd M2 = NewtonObject.Ct2List[index];
	MatrixXd mat(M1.rows(), M1.cols());
        NewtonObject.slicedCt.push_back(M1 + M2);
      }
    }
  }

  void nu(double x_) {
    NewtonObject.nu = x_;
  }

  void Nphi_u(int x_) {
    NewtonObject.Nphi_u = x_;
  }

  void Nphi_p(int x_) {
    NewtonObject.Nphi_p = x_;
  }

  void Nphi_nut(int x_) {
    NewtonObject.Nphi_nut = x_;
    NewtonObject.g_nut.resize(x_);
  }

  MatrixXd readMatrix(string const& buffer) {
    string line;
    stringstream data_stream(buffer);
    int col = 0, row = 0;
    double *buff = new double[MAXBUFSIZE];
    while(getline(data_stream, line)) {
        int temp_col = 0;
	stringstream stream(line);
        while(! stream.eof())
            stream >> buff[col*row+temp_col++];

        if (temp_col == 0)
            continue;

        if (col == 0)
            col = temp_col;
        row++;
    }

    MatrixXd matrix(row,col);
    for (int i = 0; i < row; i++)
        for (int j = 0; j < col; j++)
            matrix(i,j) = buff[ col*i+j ];

    return matrix;
  }

  void addMatrices(
      string const& P,
      string const& M,
      string const& K,
      string const& B
  ) {
    NewtonObject.P = readMatrix(P);
    NewtonObject.M = readMatrix(M);
    NewtonObject.K = readMatrix(K);
    NewtonObject.B = readMatrix(B);
  }

  void setRBF(string const& M1, string const& M2) 
  {
    MatrixXd mu = readMatrix(M1);
    MatrixXd coeffL2 = readMatrix(M2);

    NewtonObject.rbfSplines.resize(coeffL2.rows());
    for (int i = 0; i < coeffL2.rows(); i++)
    {
      SPLINTER::DataTable* samples = new SPLINTER::DataTable(1, 1);
      for (int j = 0; j < coeffL2.cols(); j++)
      {
        samples->addSample(mu.row(j), coeffL2(i, j));
      }

      NewtonObject.rbfSplines[i] = new SPLINTER::RBFSpline
      (
        *samples,
        SPLINTER::RadialBasisFunctionType::GAUSSIAN
       );
    }
  }

  void solveOnline(double vel_num) {
    VectorXd y(NewtonObject.Nphi_u + NewtonObject.Nphi_p);
    y.setZero();

    y[0] = vel_num;
    NewtonObject.BC(0) = vel_num;

    MatrixXd param(2, 1);
    param(0, 0) = vel_num;
    param(1, 0) = NewtonObject.nu;

    for (int i = 0; i < NewtonObject.Nphi_nut; i++)
    {
      NewtonObject.g_nut(i) = NewtonObject.rbfSplines[i]->eval(param);
    }

    HybridNonLinearSolver<NewtonSteady> hnls(NewtonObject);
    hnls.solve(y);
    VectorXd res(y);
    NewtonObject.operator()(y, res);

    NewtonObject.solution = y;
  }

  auto exportUnstructuredGrid() {
    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetInputData(NewtonObject.grid);
    writer->WriteToOutputStringOn();
    writer->Write();
    string binary_string = writer->GetOutputString();

    return binary_string;
  }

  auto unstructuredGridToPolyData() {
    auto geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometryFilter->SetInputData(NewtonObject.grid);
    geometryFilter->Update();

    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInputConnection(geometryFilter->GetOutputPort());
    writer->WriteToOutputStringOn();
    writer->Write();
    string binary_string = writer->GetOutputString();

    return binary_string;
  }

  auto reconstruct() {
    MatrixXd coeffU = NewtonObject.solution.block(
                             0, 0, NewtonObject.Nphi_u, 1);
    VectorXd rec = NewtonObject.modes.leftCols(NewtonObject.Nphi_u) *
                          coeffU;
    int nCellsGrid = NewtonObject.grid->GetNumberOfCells();
    vtkSmartPointer<vtkDoubleArray> velocity = vtkSmartPointer<vtkDoubleArray>::New();
    velocity->SetName ("Urec");
    velocity->SetNumberOfComponents(3);
    velocity->SetNumberOfTuples(NewtonObject.grid->GetNumberOfCells());

    for( vtkIdType i = 0; i < nCellsGrid; i++ )
    {
        double v1 = rec(i);
        double v2 = rec(i + nCellsGrid);
        double v3 = rec(i + nCellsGrid * 2);

        velocity->SetTuple3(i, v1, v2, v3);
    }
 
    vtkSmartPointer<vtkDoubleArray> velocity_points =
	    vtkSmartPointer<vtkDoubleArray>::New();
    velocity_points->SetName ("Urec");
    velocity_points->SetNumberOfComponents(3);
    velocity_points->SetNumberOfTuples(NewtonObject.grid->GetNumberOfPoints());

    NewtonObject.grid->GetCellData()->AddArray(velocity);
    NewtonObject.grid->GetPointData()->AddArray(velocity_points);

    vtkCellDataToPointData* cellToPoint = vtkCellDataToPointData::New();
    cellToPoint->ProcessAllArraysOn();
    cellToPoint->SetInputData(NewtonObject.grid);
    cellToPoint->Update();

    NewtonObject.grid = cellToPoint->GetUnstructuredGridOutput();

    auto geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometryFilter->SetInputConnection(cellToPoint->GetOutputPort());
    geometryFilter->Update();

    vtkPolyData* polydata = geometryFilter->GetOutput();
    vtkDataArray* data = polydata->GetPointData()->GetArray("Urec") ;

    int nCellsOutput = polydata->GetNumberOfPoints();

    vector<double> output(nCellsOutput * 3);

    for( vtkIdType i = 0; i < nCellsOutput; i++ )
    {
      double * values = data->GetTuple3(i);
      output.at(i) = values[0];
      output.at(i + nCellsOutput) = values[1];
      output.at(i + nCellsOutput * 2) = values[2];
    }

    emscripten::val view {
      emscripten::typed_memory_view(
        output.size(),
        output.data()
      )
    };

    auto result = emscripten::val::global("Float32Array").new_(output.size());
    result.call<void>("set", view);

    return result;
  }

  void readUnstructuredGrid(string const& buffer) {
    vtkNew<vtkXMLUnstructuredGridReader> reader;
    reader->ReadFromInputStringOn();
    reader->SetInputString(buffer);
    reader->Update();

    vtkNew<vtkUnstructuredGrid> ugrid;
    ugrid->DeepCopy(reader->GetOutput());
    NewtonObject.grid->DeepCopy(reader->GetOutput());
  }

private:
    int x;
    int y;
    NewtonSteady NewtonObject;
};

#endif // REDUCEDSTEADY_H
