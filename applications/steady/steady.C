// Copyright (C) 2022 by Carlos Peña-Monferrer
// Adapted from ITHACA-FV tutorials on https://github.com/mathLab/ITHACA-FV
// with the following copyright and license:
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
    Application for steady-state ROMs
SourceFiles
    steady.C
\*---------------------------------------------------------------------------*/

#include "steadyNS.H"
#include "SteadyNSTurb.H"
#include "ITHACAstream.H"
#include "ITHACAPOD.H"
#include "ReducedSteadyNSTurb.H"
#include "forces.H"
#include "IOmanip.H"

class pitzDaily : public SteadyNSTurb
{
    public:
        explicit pitzDaily(int argc, char* argv[])
            :
            SteadyNSTurb(argc, argv),
            U(_U()),
            p(_p()),
            nut(_nut()),
            args(_args())
        {}

        volVectorField& U;
        volScalarField& p;
        volScalarField& nut;
        argList& args;

        void offlineSolve()
        {
             ITHACAstream::read_fields(Ufield, U, "./ITHACAoutput/Offline/");
             ITHACAstream::read_fields(Pfield, p, "./ITHACAoutput/Offline/");
             ITHACAstream::read_fields(nutFields, nut, "./ITHACAoutput/Offline/");
        }
};

int main(int argc, char* argv[])
{

    argList::addOption
    (
        "Ux",
        "value",
        "Specify a Ux velocity for the online stage"
    );

    argList::addOption
    (
        "Uy",
        "value",
        "Specify a Uy velocity for the online stage"
    );

    argList::addOption
    (
        "nu",
        "value",
        "Specify a viscosity for the online stage"
    );

    argList::addBoolOption
    (
        "online",
        "Solve and reconstruct fields"
    );

    #include "setRootCase.H"

    pitzDaily example(argc, argv);

    const double UOnlineX = args.getOrDefault<scalar>("Ux", 0.0);
    const double UOnlineY = args.getOrDefault<scalar>("Uy", 0.0);
    const double nuOnline = args.getOrDefault<scalar>("nu", 1e-05);

    // label patchID = example._mesh().boundaryMesh().findPatchID("inlet");
    // const bool test = args.found("Uy");

    ITHACAparameters* para = ITHACAparameters::getInstance(example._mesh(),
                               example._runTime());
    int NmodesU = para->ITHACAdict->lookupOrDefault<int>("NmodesU", 15);
    int NmodesP = para->ITHACAdict->lookupOrDefault<int>("NmodesP", 15);
    int NmodesSUP = para->ITHACAdict->lookupOrDefault<int>("NmodesSUP", 15);
    int NmodesNut = para->ITHACAdict->lookupOrDefault<int>("NmodesNut", 15);
    int NmodesProject = para->ITHACAdict->lookupOrDefault<int>("NmodesProject", 10);

    std::ifstream parametersCount;
    parametersCount.open("ITHACAoutput/Parameters/par.txt");
    int nParameters = std::count(std::istreambuf_iterator<char>(parametersCount), 
    std::istreambuf_iterator<char>(), '\n');
    Eigen::MatrixXd par(nParameters, 3);
    std::ifstream parameters;
    parameters.open("ITHACAoutput/Parameters/par.txt");

    int k = 0;
    std::string line;
    while (std::getline(parameters, line))
    {
       std::istringstream stream(line);
       double Ux, Uy, nu;
       if (!(stream >> Ux >> Uy >> nu))
           break;
        par(k, 0) = Ux;
        par(k, 1) = Uy;
        par(k, 2) = nu;
        k++;
    }

    example.mu = par;
    // example.inletIndex.resize(0, 0);
    // example.inletIndex.resize(1, 2);
    // example.inletIndex(0, 0) = 0;
    // example.inletIndex(0, 1) = 0;
    example.inletIndex.resize(2, 2);
    example.inletIndex(0, 0) = 0;
    example.inletIndex(0, 1) = 0;
    example.inletIndex(1, 0) = 0;
    example.inletIndex(1, 1) = 1;

    example.offlineSolve();

    example.solvesupremizer();
    example.liftSolve();
    example.computeLift(example.Ufield, example.liftfield, example.Uomfield);

    ITHACAPOD::getModes(example.Uomfield, example.Umodes, example._U().name(),
                        example.podex, 0, 0, NmodesProject);
    ITHACAPOD::getModes(example.Pfield, example.Pmodes, example._p().name(),
                        example.podex, 0, 0, NmodesProject);
    ITHACAPOD::getModes(example.supfield, example.supmodes, example._U().name(),
                        example.podex, example.supex, 1, NmodesProject);
    ITHACAPOD::getModes(example.nutFields, example.nutModes, "nut",
                        example.podex, 0, 0, NmodesProject);

    example.projectSUP("./Matrices", NmodesU, NmodesP, NmodesSUP, NmodesNut);

    if (args.found("online"))
    {
        ReducedSteadyNSTurb reduced(example);
        reduced.tauU.resize(1, 1);
        Eigen::MatrixXd vel_now(2, 1);
        vel_now(0, 0) = UOnlineX;
        vel_now(1, 0) = UOnlineY;
        reduced.nu = nuOnline;
        List<Eigen::MatrixXd> EigenModes = example.L_U_SUPmodes.toEigen();
        Eigen::MatrixXd  EigenModesCoeffs = EigenModes[0].leftCols(reduced.Nphi_u);
        ITHACAstream::exportMatrix(EigenModesCoeffs, "EigenModes_U", "eigen", "./ITHACAoutput/POD/");
        reduced.tauU(0, 0) = 0;
        reduced.solveOnlineSUP(vel_now);
        Eigen::MatrixXd tmp_sol(reduced.y.rows() + 1, 1);
        tmp_sol(0) = 1;
        tmp_sol.col(0).tail(reduced.y.rows()) = reduced.y;
        reduced.online_solution.append(tmp_sol);
        reduced.rbfCoeffMat = reduced.rbfCoeff;
        reduced.reconstruct(true, "./ITHACAoutput/Reconstruction/", 1);
    }

    return 0;
}
