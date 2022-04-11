# Analysis of kernel crash dump
_[Slovak assignment](zadanie_projekt-2.txt) or [English](assignment.txt) version_

# Analysis
Let's start slowly with `crash`:
```bash
$ crash /usr/lib/debug/lib/modules/4.18.0-348.12.2.el8_5.x86_64/vmlinux vmcore_p2
```
After `crash` has loaded the crash dump it prints a couple of useful information.
The most interesting, I've skipped non-interesting ones:
```
  PANIC: "Kernel panic - not syncing: Out of memory: system-wide panic_on_oom is enabled"
    PID: 2359
COMMAND: "stress"
   TASK: ff38963f46e65ac0  [THREAD_INFO: ff38963f46e65ac0]
    CPU: 0
```

## `PANIC`
We are going to pretend, we haven't seen `COMMAND` field yet and let's just analyse the `PANIC` one.  
```
PANIC: "Kernel panic - not syncing: Out of memory: system-wide panic_on_oom is enabled"
```
One single line and it contains 2 really useful information, what a beauty:
- `Out of memory` - we got the reason, system runs out of memory. Which usually triggers [OOM Killer](https://www.kernel.org/doc/gorman/html/understand/understand016.html) but not in this case,
- The [`panic_on_oom`](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/swapping_and_out_of_memory_tips) was enabled when system runs out of memory.


## `PID`
```bash
crash> ps | grep "2359"
>  PID    PPID  CPU       TASK        ST  %MEM     VSZ    RSS  COMM
>  2359   2358   0  ff38963f46e65ac0  RU   6.5  270132 135372  stress
```
And here is the perpetrator, the `2359` pid of `stress` process.

## `CPU`
Nothing surprising, the CPU ID that panics the kernel we can also get from `ps` command (there is CPU that 
was running guilty PID).


## Running process
As we can see from `PID` and also from `COMMAND` the [`stress`](https://linux.die.net/man/1/stress).

## Memory
As we know from analysis of `PANIC` field, OOM causes panic. So let's analyse the memory:
```bash
crash> kmem -i
                 PAGES        TOTAL      PERCENTAGE
    TOTAL MEM   464508       1.8 GB         ----
         FREE    12961      50.6 MB    2% of TOTAL MEM
         USED   451547       1.7 GB   97% of TOTAL MEM
       SHARED     1615       6.3 MB    0% of TOTAL MEM
      BUFFERS        0            0    0% of TOTAL MEM
       CACHED     2265       8.8 MB    0% of TOTAL MEM
         SLAB     8339      32.6 MB    1% of TOTAL MEM
                              ...
 COMMIT LIMIT   232254     907.2 MB         ----
    COMMITTED   607729       2.3 GB  261% of TOTAL LIMIT
```
_`SWAP` and `HUGE` sections are skipped, nothing interesting there._

The `stress` was probably running the RAM stress test.  
As we can see from `USED` and `FREE` fields, 97% of total system memory
was already allocated and only 2% was available (~50 MB).  
That **probably** means, `stress` tried to allocate block bigger than `50.6 MB` which, was not available and system panics
due to that.

## Backtracking
We used `-l` argument to let `bt` help us by inserting information from source code.
```bash
crash> bt -fl
PID: 2359   TASK: ff38963f46e65ac0  CPU: 0   COMMAND: "stress"
 #0 [ff41a7a440acfa38] machine_kexec at ffffffff9e6635ce
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/arch/x86/kernel/machine_kexec_64.c: 389
    ff41a7a440acfa40: 0000803411191500 ff38963f40000000
    ff41a7a440acfa50: 0000000075003000 ff38963fb5003000
    ff41a7a440acfa60: 0000000075002000 fffa322300040800
    ff41a7a440acfa70: 433f803411191500 0000000000000000
    ff41a7a440acfa80: ff41a7a440acfa98 ffffffff9f6e9990
    ff41a7a440acfa90: ffffffff9e79d6bd
 #1 [ff41a7a440acfa90] __crash_kexec at ffffffff9e79d6bd
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/kernel/kexec_core.c: 957
    ff41a7a440acfa98: ff41a7a440acfd18 0000000000000001
    ff41a7a440acfaa8: 0000000000000003 ffffffff9f6e9990
    ff41a7a440acfab8: ff41a7a440acfbd0 0000000000000000
    ff41a7a440acfac8: 0000000000000001 0000000000000000
    ff41a7a440acfad8: 0000000000aaaaaa ffffffff9fc07b80
    ff41a7a440acfae8: 0000000000000001 ff38963f46e65ac0
    ff41a7a440acfaf8: 0000000000000000 ff38963f46e65ac0
    ff41a7a440acfb08: ffffffff9fcb4a40 0000000000000003
    ff41a7a440acfb18: ffffffff9e79d75f 0000000000000010
    ff41a7a440acfb28: 0000000000000046 ff41a7a440acfa98
    ff41a7a440acfb38: 0000000000000000 433f803411191500
    ff41a7a440acfb48: ff41a7a440acfc00 ff41a7a440acfbd0
    ff41a7a440acfb58: ffffffff9e6eb227
 #2 [ff41a7a440acfb58] panic at ffffffff9e6eb227
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/./arch/x86/include/asm/smp.h: 72
    ff41a7a440acfb60: 0000000000000010 ff41a7a440acfbe0
    ff41a7a440acfb70: ff41a7a440acfb80 433f803411191500
    ff41a7a440acfb80: 000000000000a187 ffffffff9f6e93bb
    ff41a7a440acfb90: 0000000000000000 0000000000000000
    ff41a7a440acfba0: 00000000000002e8 0000000000aaaaaa
    ff41a7a440acfbb0: ff41a7a440acfc98 ff38963fbffb3ce0
    ff41a7a440acfbc0: 0000000000000003 0000000000000001
    ff41a7a440acfbd0: 0000000000000000 ffffffff9e87e6f1
 #3 [ff41a7a440acfbd8] out_of_memory.cold.35 at ffffffff9e87e6f1
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/mm/oom_kill.c: 1088
    ff41a7a440acfbe0: 0000000000000000 433f803411191500
    ff41a7a440acfbf0: 0000000000000000 0000000000000713
    ff41a7a440acfc00: ff38963f46e65ac0 0000000000000000
    ff41a7a440acfc10: 00000000006280ca ffffffff9e8d4825
 #4 [ff41a7a440acfc18] __alloc_pages_slowpath at ffffffff9e8d4825
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/mm/page_alloc.c: 4056
    ff41a7a440acfc20: 0000000000000240 0000000000000000
    ff41a7a440acfc30: 0000000000000000 0000000000000060
    ff41a7a440acfc40: 0000000000000073 0000000000000000
    ff41a7a440acfc50: 0000000000000000 0000000100000240
    ff41a7a440acfc60: 0020000000000000 00000000006280ca
    ff41a7a440acfc70: 00000000004280ca 0040000000000010
    ff41a7a440acfc80: 0000004000000000 0000005000000010
    ff41a7a440acfc90: 00000000000000f0 ff38963fbffb3cc0
    ff41a7a440acfca0: 0000000000000000 0000000000000000
    ff41a7a440acfcb0: 00000000006280ca 000000000007167c
    ff41a7a440acfcc0: 0000000000000000 0000000000000000
    ff41a7a440acfcd0: 0000000000000000 433f803411191500
    ff41a7a440acfce0: 00000000006280ca 0000000000000000
    ff41a7a440acfcf0: 0000000000000000 00000000006280ca
    ff41a7a440acfd00: 0000000000000000 0000000000000000
    ff41a7a440acfd10: ffffffff9e8d4beb
 #5 [ff41a7a440acfd10] __alloc_pages_nodemask at ffffffff9e8d4beb
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/mm/page_alloc.c: 4949
    ff41a7a440acfd18: ff38963fbffb3cc0 0000000000000000
    ff41a7a440acfd28: ff38963fbffb3cc0 0000000300000001
    ff41a7a440acfd38: 0000000000000000 433f803411191500
    ff41a7a440acfd48: ffffffffa06d73e0 00000000006280ca
    ff41a7a440acfd58: 0000000000000000 0000000000000000
    ff41a7a440acfd68: ff38963f52bee2b8 ffffffff9e8ef414
 #6 [ff41a7a440acfd70] alloc_pages_vma at ffffffff9e8ef414
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/./include/linux/mempolicy.h: 83
    ff41a7a440acfd78: ff38963f00000000 ff41a7a440acfdf8
    ff41a7a440acfd88: ff38963f52bee2b8 0000000000000001
    ff41a7a440acfd98: 0000000000000001 00007fb14644e010
    ff41a7a440acfda8: ff38963f740db180 ffffffff9e8b1077
 #7 [ff41a7a440acfdb0] do_anonymous_page at ffffffff9e8b1077
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/./include/linux/topology.h: 87
    ff41a7a440acfdb8: 0000000000000270 000000000384c067
    ff41a7a440acfdc8: 0000000000000190 0000000000000001
    ff41a7a440acfdd8: 00007fb14644e010 ff38963f740db180
    ff41a7a440acfde8: ffffffff9e8b7336
 #8 [ff41a7a440acfde8] __handle_mm_fault at ffffffff9e8b7336
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/mm/memory.c: 4141
    ff41a7a440acfdf0: ffffffff9f0001f1 ff38963f52bee2b8
    ff41a7a440acfe00: 006000c000000255 00000007fb14644e
    ff41a7a440acfe10: 00007fb14644e000 ff38963f46bc5190
    ff41a7a440acfe20: ff38963f433ab628 0000000000000000
    ff41a7a440acfe30: 0000000000000000 0000000000000000
    ff41a7a440acfe40: 0000000000000000 0000000000000000
    ff41a7a440acfe50: 0000000000000000 0000000000000000
    ff41a7a440acfe60: 433f803411191500 0000000000000255
    ff41a7a440acfe70: ff38963f52bee2b8 00007fb14644e010
    ff41a7a440acfe80: 0000000000000040 ff38963f740db180
    ff41a7a440acfe90: ff38963f46e65ac0 ffffffff9e8b742e
 #9 [ff41a7a440acfe98] handle_mm_fault at ffffffff9e8b742e
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/mm/memory.c: 4313
    ff41a7a440acfea0: 0000000000000006 ff41a7a440acff58
    ff41a7a440acfeb0: 00007fb14644e010 0000000000000255
    ff41a7a440acfec0: ffffffff9e674f5d
#10 [ff41a7a440acfec0] __do_page_fault at ffffffff9e674f5d
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/./include/linux/sched/signal.h: 397
    ff41a7a440acfec8: ff38963f740db1f0 0000000000000000
    ff41a7a440acfed8: 0000000000000000 0000000000000002
    ff41a7a440acfee8: ff38963f46e65ac0 0000000000000000
    ff41a7a440acfef8: ff41a7a440acff58 00007fb14644e010
    ff41a7a440acff08: 0000000000000006 0000000000000000
    ff41a7a440acff18: 0000000000000000 ffffffff9e675267
#11 [ff41a7a440acff20] do_page_fault at ffffffff9e675267
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/./arch/x86/include/asm/jump_label.h: 38
    ff41a7a440acff28: 0000000000000000 0000000000000000
    ff41a7a440acff38: ffffffff9f001108 0000000000000000
    ff41a7a440acff48: 0000000000000000 ffffffff9f00111e
#12 [ff41a7a440acff50] page_fault at ffffffff9f00111e
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/arch/x86/entry/entry_64.S: 1224
    RIP: 000056525428d210  RSP: 00007ffe41cb9f40  RFLAGS: 00010206
    RAX: 000000000841a000  RBX: 0000000000000000  RCX: 00007fb13e034010
    RDX: 0000000000000000  RSI: 0000000010001000  RDI: 0000000000000000
    RBP: 00007fb13e034010   R8: 00000000ffffffff   R9: 0000000000000000
    R10: 0000000000000022  R11: 0000000000000246  R12: 0000000000001000
    R13: 000056525428f004  R14: 0000000000000002  R15: 0000000010000000
    ORIG_RAX: ffffffffffffffff  CS: 0033  SS: 002b
```
Let's analyse it deeper:    
Regards to [`do_page_fault` documentation](https://elixir.bootlin.com/linux/v4.18/source/arch/x86/mm/fault.c#L1212)
and partially also from:
```bash
crash> whatis __do_page_fault
void __do_page_fault(struct pt_regs *, unsigned long, unsigned long);
crash> whatis handle_mm_fault
vm_fault_t handle_mm_fault(struct vm_area_struct *, unsigned long, unsigned int);
```
The requested address is 3rd parameter of `__do_page_fault()` and 2nd parameter of `handle_mm_fault`.  
These parameters would be stored in `%rdx` and `%rcx` registers. As we know, the content of these registers 
we can find also at the stack.
So, using the stack of these functions:
```bash
 #9 [ff41a7a440acfe98] handle_mm_fault at ffffffff9e8b742e
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/mm/memory.c: 4313
    ff41a7a440acfea0: 0000000000000006 ff41a7a440acff58
    ff41a7a440acfeb0: 00007fb14644e010 0000000000000255
    ff41a7a440acfec0: ffffffff9e674f5d
#10 [ff41a7a440acfec0] __do_page_fault at ffffffff9e674f5d
    /usr/src/debug/kernel-4.18.0-348.12.2.el8_5/linux-4.18.0-348.12.2.el8_5.x86_64/./include/linux/sched/signal.h: 397
    ff41a7a440acfec8: ff38963f740db1f0 0000000000000000
    ff41a7a440acfed8: 0000000000000000 0000000000000002
    ff41a7a440acfee8: ff38963f46e65ac0 0000000000000000
    ff41a7a440acfef8: ff41a7a440acff58 00007fb14644e010
    ff41a7a440acff08: 0000000000000006 0000000000000000
```
We can see, that address is `00007fb14644e010`.

# Conclusion
We found out, the `stress` process was running when kernel panic happened. Unfortunately, kernel dump does not contain
user-space memory dump, so we cannot find out exact `stress` command arguments. But we can assume, it was running some
RAM stress test.  
From the `ps` command, we can see, the `stress` was running at several CPUs but `CPU 0` tried to allocate or access
memory at `00007fb14644e010`. Address wasn't present (page fault), so kernel tried to allocate
a space for that page, but system had not enough space for it.  
Usually OOM killer would be started, but in this case,
the `panic_on_oom` was enabled, so system panics.

---
#### Panic avoiding
Avoiding to this panic is pretty easy, just don't run RAM stress test with `panic_on_oom` enabled. If `panic_on_oom` 
was disabled, kernel could start OOM killer and could easily kill one of running process instead of panics.

---
Author: Samuel Dobron (xdobro23)