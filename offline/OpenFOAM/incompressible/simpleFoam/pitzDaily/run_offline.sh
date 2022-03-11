#!/bin/bash

. ${WM_PROJECT_DIR:?}/bin/tools/CleanFunctions 

U_min=0.5
U_max=20
U_step=0.5

nu_min=1e-06
nu_max=1e-04
nu_step=2.475e-05

offline_path="ITHACAoutput/Offline"
param_path="ITHACAoutput/Parameters"

echo $param_path

rm -r ITHACAoutput
mkdir -p ${offline_path}
mkdir -p ${param_path}

echo -n > ${param_path}/par.txt

ln -s ../../0 $offline_path/0
ln -s ../../constant $offline_path/constant
ln -s ../../system $offline_path/system

counter=1

for U in $(seq $U_min $U_step $U_max); do
  for nu in $(seq $nu_min $nu_step $nu_max); do
    echo "Running ${counter}: U $U and nu $nu..."
    echo "$U $nu" >> ${param_path}/par.txt

    foamDictionary constant/transportProperties -entry nu -set $nub >/dev/null
    foamDictionary 0/U -entry boundaryField.inlet.value -set "uniform ($U 0 0)" >/dev/null
    simpleFoam > log.simpleFoam_$counter.log 2>&1

    results_dir=$(foamListTimes -latestTime)
    cp -r ${results_dir} ${offline_path}/${counter}
    counter=$((counter+1))
    cleanTimeDirectories >/dev/null
  done
done

touch ${offline_path}/foam.foam
