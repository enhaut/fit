# SystemTap probes
## Assignment
TODO

## Usage
Script requires root privileges and one argument - name of process to measure.
No signals are blocked/masked so it could be easily interrupted for example with `SIGINT`.
```bash
$ stap proj4.stap [TARGET_PROCESS]
```
Example:
```bash
$ stap proj4.stap stress
Watching process: stress
PID 289122 executed execve syscall for pathname: ./stress
PID 289123 executed execve syscall for pathname: ./stress

================================================
Scheduling stats:
CPU	PID	run	hrtimer		comm
1	289122	  0	      0 us	stress
0	289123	 86	   9700 us	stress
================================================
^C
================================================
Scheduling stats:
CPU	PID	run	hrtimer		comm
1	289122	  0	      0 us	stress
0	289123	 50	   5484 us	stress
================================================
Bye...
```
Developed and tested for kernel version `4.18.0-348.12.2.el8_5.x86_64`.
## Documentation
There are couple of `global` variables:
- `target_process` - name of process to watch
  - Process is being watched also for cases, when `target_process` is just substring of process name
- `run_times` nested (3D) array indexable by `cpuid`,  `pid` and key:
  - `start` process execution start timestamp
  - `state` current state of process
    - `1` process is running
    - `0` process is sleeping/has been killed
  - `hr_start` timestamp of entering the interrupt handler
  - `initialized` used to mark whether `times[cpuid, pid]` and `hrtimes[cpuid, pid]` has been initialized
- `times` nested (2D) array indexable by `cpuid` and `pid` used to store execution times of processes
- `hrtimes` nested (2D) array indexable by `cpuid` and `pid` used to store execution times of interrupts handling
- `names` nested (2D) array also indexable by `cpuid, pid` used to store process name

### Probes
Probe `nd_syscall.clone` is used to watch `clone()`, `fork()` and `vfork()` syscalls by `TARGET_PROCESS`.
To monitor `execve()` and `execveat()` syscalls, probe is attached to the `kernel.function("do_execve")` function.  
  
Measuring execution times of process are calculated by attaching probe to the `scheduler.cpu_on`. Whenever kernel enters 
`__schedule()` function timestamp is saved to `run_times[cpuid, pid, "start"]`. Execution time is then calculated when 
kernel enters `__schedule()` execution time is calculated using formula `gettimeofday_us() - run_times[cpuid, pid, "start"]` 
and saved to the `times[cpuid, pid]`.  
  
Execution times of interrupts handler are measured by probes `kernel.function("hrtimer_interrupt@kernel/time/hrtimer.c")`
and `kernel.function("hrtimer_interrupt@kernel/time/hrtimer.c").return`. Whenever process is interrupted, timestamp is 
saved to `run_times[cpuid, pid, "hr_start"]`, probe `.return` is saves execution time to the `hr_times[cpuid, pid]`.

---
Author: Samuel Dobron (xdobro23), FIT BUT
