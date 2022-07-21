#ifndef VTK_H
#define VTK_H

#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <list>

#include <vector>

#include <vtkGeometryFilter.h>
#include <vtkNew.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkSmartPointer.h>
#include <vtkCellDataToPointData.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

class VTK {

public:

  VTK(){
    vtkObject::GlobalWarningDisplayOff();
  };

  void test() {
  }

  void readUnstructuredGrid(std::string const& buffer) {
    vtkNew<vtkXMLUnstructuredGridReader> reader;
    reader->ReadFromInputStringOn();
    reader->SetInputString(buffer);
    reader->Update();

    grid->DeepCopy(reader->GetOutput());
  }

  auto planeX(double x) {
    dynPlane->SetOrigin(x, 0, 0);
    dynPlane->SetNormal(1, 0, 0);

    cutter->SetCutFunction(dynPlane);
    cutter->SetInputData(grid);
    cutter->Update();

    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInputConnection(cutter->GetOutputPort());
    writer->WriteToOutputStringOn();
    writer->Write();

    std::string binary_string = writer->GetOutputString();

    return binary_string;
  }

  auto planeY(double x) {
    dynPlane->SetOrigin(0, x, 0);
    dynPlane->SetNormal(0, 1, 0);

    cutter->SetCutFunction(dynPlane);
    cutter->SetInputData(grid);
    cutter->Update();

    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInputConnection(cutter->GetOutputPort());
    writer->WriteToOutputStringOn();
    writer->Write();
    std::string binary_string = writer->GetOutputString();

    return binary_string;
  }

  auto planeZ(double x) {
    dynPlane->SetOrigin(0, 0, x);
    dynPlane->SetNormal(0, 0, 1);

    cutter->SetCutFunction(dynPlane);
    cutter->SetInputData(grid);
    cutter->Update();

    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInputConnection(cutter->GetOutputPort());
    writer->WriteToOutputStringOn();
    writer->Write();
    std::string binary_string = writer->GetOutputString();

    return binary_string;
  }

  auto unstructuredGridToPolyData() {
    vtkNew<vtkGeometryFilter> geometryFilter;
    geometryFilter->SetInputData(grid);
    geometryFilter->Update();

    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInputConnection(geometryFilter->GetOutputPort());
    writer->WriteToOutputStringOn();
    writer->Write();
    std::string binary_string = writer->GetOutputString();

    return binary_string;
  }

private:
  vtkSmartPointer<vtkUnstructuredGrid> grid =
    vtkSmartPointer<vtkUnstructuredGrid>::New();
  vtkNew<vtkPlane> dynPlane;
  vtkNew<vtkCutter> cutter;
};

#endif // VTK_H
