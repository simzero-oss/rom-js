// Copyright (c) 2022 Carlos Peña-Monferrer. All rights reserved.
// This work is licensed under the terms of the GNU LGPL v3.0 license.
// For a copy, see <https://opensource.org/licenses/LGPL-3.0>.

import rom from '@simzero/rom'
import Papa from 'papaparse'
import fs from 'fs'

let caseName = 'pitzDaily'
let Ux = 3.0;
let Uy = 0.0;
let nu = 1e-05;

if (process.argv[2])
{
  caseName = process.argv[2];
  Ux = process.argv[3];
  Uy = process.argv[4];
  nu = process.argv[5];
}

const rootPath = '../../surrogates/OF/incompressible/simpleFoam/' + caseName + "/";
const outputFile = caseName + '_U_' + Ux + '_' + Uy + '_nu_' + nu + '.vtu';

const loadData = async (dataPath) => {
  const data = await readFile(rootPath + dataPath)
  const vector = dataToVector(data);
  return vector;
}

const readFile = async (filePath) => {
  const csvFile = fs.readFileSync(filePath)
  const csvData = csvFile.toString()
  return new Promise(resolve => {
    Papa.parse(csvData, {
      delimiter: ' ',
      dynamicTyping: true,
      skipEmptyLines: true,
      header: false,
      complete: results => {
        resolve(results.data);
      }
    });
  });
};

const dataToVector = (data) => {
  const vecVec = new rom.VectorVector();
  let rows = 0;
  let cols = 0;
  data.forEach(row => {
    var vec = new rom.Vector();
    row.forEach(value => {
      vec.push_back(value)
      if (rows === 0)
        cols++;
    });
    vecVec.push_back(vec)
    rows++;
  })
  return [vecVec, rows, cols];
}

(async () => {
  var startInitialize = new Date().getTime();

  await rom.ready

  const N_BC = 2;

  const P = await loadData('matrices/P_mat.txt');
  const M = await loadData('matrices/M_mat.txt');
  const K = await loadData('matrices/K_mat.txt');
  const B = await loadData('matrices/B_mat.txt');
  const modes = await loadData('EigenModes_U_mat.txt');

  const coeffL2 = await loadData('matrices/coeffL2_mat.txt');
  const mu = await loadData('par.txt');
  const grid_data = fs.readFileSync(rootPath + caseName + '.vtu')

  const Nphi_u = B[1];
  const Nphi_p = K[2];
  const Nphi_nut = coeffL2[1];

  const reduced = new rom.reducedSteady(Nphi_u + Nphi_p, Nphi_u + Nphi_p);

  reduced.readUnstructuredGrid(grid_data);

  reduced.Nphi_u(Nphi_u);
  reduced.Nphi_p(Nphi_p);
  reduced.Nphi_nut(Nphi_nut);	
  reduced.N_BC(N_BC);
  reduced.addMatrices(P[0], M[0], K[0], B[0]);
  reduced.addModes(modes[0]);
  reduced.setRBF(mu[0], coeffL2[0]);
  reduced.nu(nu);

  for (var i = 0; i < Nphi_u; i ++ ){
    const C = await loadData('matrices/C' + i + '_mat.txt');
    const Ct1 = await loadData('matrices/ct1_' + i + '_mat.txt');
    const Ct2 = await loadData('matrices/ct2_' + i + '_mat.txt');

    reduced.addCMatrix(C[0], i);
    reduced.addCt1Matrix(Ct1[0], i);
    reduced.addCt2Matrix(Ct2[0], i);
  }

  reduced.preprocess();

  var endInitialize = new Date().getTime();
  var timeInitialize = endInitialize - startInitialize;

  var startSolve = new Date().getTime();
  reduced.solveOnline(Ux, Uy);
  reduced.reconstruct();
  var endSolve = new Date().getTime();
  var timeSolve = endSolve - startSolve;

  const dataString = reduced.exportUnstructuredGrid();

  fs.writeFileSync(outputFile, dataString)

  console.log('Execution time initialize: ' + 0.001*timeInitialize + " s.");
  console.log('Execution time solve and reconstruct: ' + 0.001*timeSolve + " s.");

  reduced.delete();

})();
