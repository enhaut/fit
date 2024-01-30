#!/bin/bash
#SBATCH --job-name          avs_lab5_advi
#SBATCH --account           DD-23-135
#SBATCH --partition         qcpu_exp
#SBATCH --nodes             1
#SBATCH --ntasks-per-node   36
#SBATCH --time              0:05:00
#SBATCH --output=%x.ID-%j.out
#SBATCH --error=%x.ID-%j.err


ml Advisor intel

# build and run
make clean
make  > make.log

# Advisor
rm -rf Advisor
mkdir Advisor

# Basic survey
advixe-cl -collect survey -project-dir Advisor  -- ./lab5

# Roof line
advixe-cl -collect tripcounts -flop -project-dir Advisor  -- ./lab5