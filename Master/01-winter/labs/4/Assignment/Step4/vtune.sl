#!/bin/bash
#SBATCH --job-name          avs_lab4_vtune
#SBATCH --account           DD-23-135
#SBATCH --partition         qcpu_exp
#SBATCH --nodes             1
#SBATCH --ntasks-per-node   36
#SBATCH --time              0:05:00
#SBATCH --comment           use:vtune=2022.2.0
#SBATCH --output=%x.ID-%j.out
#SBATCH --error=%x.ID-%j.err

ml VTune intel

# build and run
make clean
make  > make.log

# VTune
rm -rf VTune
mkdir VTune

# HotSpot
vtune -collect hotspots -knob sampling-mode=hw -knob sampling-interval=1 -r VTune/lab4/hotspots  -app-working-dir . -- ./lab4

# Microarchitecture
vtune -collect uarch-exploration -knob sampling-interval=1 -knob collect-memory-bandwidth=true -r VTune/lab4/uarch -app-working-dir . -- ./lab4

#Memory access
vtune -collect memory-access -knob sampling-interval=1 -knob analyze-mem-objects=true -r VTune/lab4/ma -app-working-dir . -- ./lab4

# HPC
vtune -collect hpc-performance -knob sampling-interval=1 -knob collect-memory-bandwidth=true -knob collect-affinity=true-r VTune/lab4/hpc -app-working-dir . -- ./lab4

# Threading
vtune -collect threading -knob sampling-and-waits=hw -knob sampling-interval=1 -knob enable-stack-collection=true -knob stack-size=2048 -r VTune/lab4/thr -app-working-dir . -- ./lab4
