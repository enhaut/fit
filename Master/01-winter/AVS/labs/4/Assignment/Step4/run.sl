#!/bin/bash
#SBATCH --job-name          avs_lab4_run
#SBATCH --account           DD-23-135
#SBATCH --partition         qcpu_exp
#SBATCH --nodes             1
#SBATCH --ntasks-per-node   36
#SBATCH --time              0:05:00
#SBATCH --output=%x.ID-%j.out
#SBATCH --error=%x.ID-%j.err

ml intel

# build and run
make clean
make  > make.log
./lab4 > run.log
