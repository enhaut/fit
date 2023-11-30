#!/bin/bash
#SBATCH --job-name          avs_lab5_run
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
OMP_NUM_THREADS=1 ./lab5  > run.log
OMP_NUM_THREADS=2 ./lab5  >> run.log 
OMP_NUM_THREADS=4 ./lab5  >> run.log 
OMP_NUM_THREADS=8 ./lab5  >> run.log 
OMP_NUM_THREADS=16 ./lab5 >> run.log 
OMP_NUM_THREADS=24 ./lab5 >> run.log 
OMP_NUM_THREADS=36 ./lab5 >> run.log 
