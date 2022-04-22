#/bin/bash
#
# Copyright (c) 2022 Carlos Peña-Monferrer. All rights reserved.
# This work is licensed under the terms of the GNU LGPL v3.0 license.
# For a copy, see <https://opensource.org/licenses/LGPL-3.0>.
#

run_case () {
  mkdir -p runs/$k/constant && \
  cp -r 0 runs/$k/ && \
  cp inputs runs/$k/ && \
  cp constant/transportProperties runs/$k/constant/ && \
  cd runs/$k/constant && \
  ln -s ../../../constant/polyMesh polyMesh && \
  ln -s ../../../constant/turbulenceProperties turbulenceProperties && \
  cd .. && \
  ln -s ../../system system && \
  foamDictionary constant/transportProperties -entry nu -set ${nu_params[$k]} && \
  foamDictionary inputs -entry U -set ${U_params[$k]} -disableFunctionEntries && \
  foamJob -wait simpleFoam && \
  results_dir=$(foamListTimes -latestTime) && \
  output_dir=$(($k + 1))
  cd ../../ITHACAoutput/Offline && \
  ln -s ../../runs/$k/${results_dir} ${output_dir}
}

cores=$1

U_min=2.0
U_max=90.0
U_step=1.0

nu_min=6.0e-06
nu_max=1.8e-05
nu_step=2.0e-06

offline_path="ITHACAoutput/Offline"
param_path="ITHACAoutput/Parameters"

if [ ! -d runs ];then
  mkdir -p runs
fi

if [ ! -d ${param_path} ];then
  mkdir -p ${param_path}
fi

if [ ! -d ${offline_path} ];then
  mkdir -p ${offline_path}
fi

ln -s ../../0 $offline_path/0
ln -s ../../constant $offline_path/constant
ln -s ../../system $offline_path/system

n_params=0
for U in $(seq $U_min $U_step $U_max); do
  for nu in $(seq $nu_min $nu_step $nu_max); do
    let "n_params=n_params+1"
    U_params=( "${U_params[@]}" "$U" )
    nu_params=( "${nu_params[@]}" "$nu" )
  done
done

echo -n > $param_path/par.txt

# The following executes a number single cores runs in parallel determined by
# ${cores}. It is a straightforward way of running the workload for the
# tutorials but other solutions might be more efficient in HPC environments.
k=0
while [ $k -lt $n_params ]
do
  for i in `seq ${cores}`
  do
    if [ $k -lt $n_params ]; then
      echo "Running $k with U ${U_params[$k]} and nu ${nu_params[$k]}"
      (run_case $k ${U_params[$k]} ${nu_params[$k]}) &
      echo "${U_params[$k]} 0.0 ${nu_params[$k]}" >> $param_path/par.txt
      let "k=k+1"
    fi
  done
  wait
done

touch ITHACAoutput/Offline/foam.foam
