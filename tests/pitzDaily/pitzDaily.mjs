import rom from 'rom'

import fs from 'fs'

let U = 3.0;
let nu = 1e-05;

if(process.argv[2])
{
  U = process.argv[2];
  nu = process.argv[3];
}

const outputFile = 'U_' + U + '_nu_' + nu + '.vtu';

(async () => {
  var startInitialize = new Date().getTime();

  await rom.ready

  const N_BC = 1;

  const P = fs.readFileSync("data/matrices/P_mat.txt");
  const M = fs.readFileSync("data/matrices/M_mat.txt");
  const K = fs.readFileSync("data/matrices/K_mat.txt");
  const B = fs.readFileSync("data/matrices/B_mat.txt");
  const modes = fs.readFileSync('data/EigenModes_U_mat.txt');
  const coeffL2 = fs.readFileSync('data/matrices/coeffL2_mat.txt');
  const mu = fs.readFileSync('data/par.txt');
  const grid_data = fs.readFileSync('data/pitzDaily.vtu')

  const Nphi_u = B.split("\n").length;
  const Nphi_p = K.split("\n")[0].split(" ").length;
  const Nphi_nut = coeffL2.split("\n").length;


  for (var i = 0; i < Nphi_u; i ++ ){	
    const C = fs.readFileSync("data/matrices/C" + i + "_mat.txt");
    const Ct1 = fs.readFileSync("data/matrices/ct1_" + i + "_mat.txt");
    const Ct2 = fs.readFileSync("data/matrices/ct2_" + i + "_mat.txt");

    reduced.addCMatrix(C, i);
    reduced.addCt1Matrix(Ct1, i);
    reduced.addCt2Matrix(Ct2, i);
  }

  const reduced = new rom.reducedSteady(Nphi_u + Nphi_p, Nphi_u + Nphi_p);

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
