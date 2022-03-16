// Copyright (c) 2022 Carlos Peña-Monferrer. All rights reserved.
// This work is licensed under the terms of the GNU LGPL v3.0 license.
// For a copy, see <https://opensource.org/licenses/LGPL-3.0>.

import rom from '@simzero/rom'
import fs from 'fs'

let U = 3.0;
let nu = 1e-05;

if(process.argv[2])
{
  U = process.argv[2];
  nu = process.argv[3];
}

const dataPath = "../../surrogates/OF/incompressible/simpleFoam/pitzDaily/";
const outputFile = 'U_' + U + '_nu_' + nu + '.vtu';

const loadData = (path) => {
  const data = fs.readFileSync(path).toString();
  return data;
}

(async () => {
  var startInitialize = new Date().getTime();

  await rom.ready

  const N_BC = 1;

  const P = loadData(dataPath + "matrices/P_mat.txt");
  const M = loadData(dataPath + "matrices/M_mat.txt");
  const K = loadData(dataPath + "matrices/K_mat.txt");
  const B = loadData(dataPath + "matrices/B_mat.txt");
  const modes = loadData(dataPath + "EigenModes_U_mat.txt");
  const coeffL2 = loadData(dataPath + "matrices/coeffL2_mat.txt");
  const mu = loadData(dataPath + "par.txt");
  const grid_data = fs.readFileSync(dataPath + "pitzDaily.vtu")

  const Nphi_u = B.split("\n").length;
  const Nphi_p = K.split("\n")[0].split(" ").length;
  const Nphi_nut = coeffL2.split("\n").length;

  const reduced = new rom.reducedSteady(Nphi_u + Nphi_p, Nphi_u + Nphi_p);

  for (var i = 0; i < Nphi_u; i ++ ){	
    const C = loadData(dataPath + "matrices/C" + i + "_mat.txt");
    const Ct1 = loadData(dataPath + "matrices/ct1_" + i + "_mat.txt");
    const Ct2 = loadData(dataPath + "matrices/ct2_" + i + "_mat.txt");

    reduced.addCMatrix(C, i);
    reduced.addCt1Matrix(Ct1, i);
    reduced.addCt2Matrix(Ct2, i);
  }


  reduced.readUnstructuredGrid(grid_data);

  reduced.Nphi_u(Nphi_u);
  reduced.Nphi_p(Nphi_p);
  reduced.Nphi_nut(Nphi_nut);	
  reduced.N_BC(N_BC);
  reduced.addMatrices(P, M, K, B);
  reduced.addModes(modes);
  reduced.preprocess();

  reduced.setRBF(mu, coeffL2);
  reduced.nu(nu);

  var endInitialize = new Date().getTime();
  var timeInitialize = endInitialize - startInitialize;

  var startSolve = new Date().getTime();
  reduced.solveOnline(U);
  reduced.reconstruct();
  var endSolve = new Date().getTime();
  var timeSolve = endSolve - startSolve;

  const dataString = reduced.exportUnstructuredGrid();

  fs.writeFileSync(outputFile, dataString)

  console.log('Execution time initialize: ' + 0.001*timeInitialize + " s.");
  console.log('Execution time solve and reconstruct: ' + 0.001*timeSolve + " s.");

  reduced.delete();

})();
