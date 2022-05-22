# Analysis of kernel crash dump
_[Slovak assignment](zadanie_projekt-3.txt) or [English](assignment.txt) version_

# Analysis
First, let's check the logs, to see what actually happened.
```bash
crash> log | tail -n 53 | head -n 5
[   51.555322] smajdalf: loading out-of-tree module taints kernel.
[   51.555903] smajdalf: module license 'RH-EDU' taints kernel.
[   51.556426] Disabling lock debugging due to kernel taint
[   51.556949] smajdalf: module verification failed: signature and/or required key missing - tainting kernel
[   51.558281] Smajdalf: Carodej nikdy nechodi pozde.
```
Hmm, as we can see, the kernel was tainted, that **probably** means, there is an issue
with loaded module - `smajdalf`.

About 190 seconds later the panic happened.
```bash
crash> log | tail -n 48
[  245.808706] INFO: task systemd:1 blocked for more than 120 seconds.
[  245.809290]       Tainted: P           OE    --------- -  - 4.18.0-348.12.2.el8_5.x86_64 #1
[  245.810057] "echo 0 > /proc/sys/kernel/hung_task_timeout_secs" disables this message.
[  245.810779] task:systemd         state:D stack:    0 pid:    1 ppid:     0 flags:0x00000000
[  245.811520] Call Trace:
[  245.811758]  __schedule+0x2bd/0x760
[  245.812076]  schedule+0x37/0xa0
[  245.812363]  rwsem_down_read_slowpath+0x360/0x3d0
[  245.812802]  __do_page_fault+0x3b1/0x4c0
[  245.813157]  do_page_fault+0x37/0x130
[  245.813493]  ? page_fault+0x8/0x30
[  245.813812]  page_fault+0x1e/0x30
[  245.814117] RIP: 0033:0x7f2411c4e27f
[  245.814452] Code: Unable to access opcode bytes at RIP 0x7f2411c4e255.
[  245.815041] RSP: 002b:00007fff58f62300 EFLAGS: 00010206
[  245.815515] RAX: 000055c2b6810bf0 RBX: 00007f2411f89bc0 RCX: 000055c2b67f67c0
[  245.816155] RDX: 00007f2411f89c50 RSI: 00007f2411f89c40 RDI: 00007f2411f89bc8
[  245.816798] RBP: 000000000000001d R08: 000055c2b66515f0 R09: 000055c2b66515e0
[  245.817433] R10: 00007f2411f89bc0 R11: 0000000000000007 R12: 0000000000000003
[  245.818075] R13: 000000000000001d R14: 00007f2411f89bc0 R15: 0000000000000030
[  245.818753] NMI backtrace for cpu 2
[  245.819072] CPU: 2 PID: 42 Comm: khungtaskd Kdump: loaded Tainted: P           OE    --------- -  - 4.18.0-348.12.2.el8_5.x86_64 #1
[  245.820128] Hardware name: Red Hat Container-native virtualization/RHEL-AV, BIOS 1.14.0-1.module+el8.4.0+8855+a9e237a9 04/01/2014
[  245.821168] Call Trace:
[  245.821402]  dump_stack+0x5c/0x80
[  245.821713]  nmi_cpu_backtrace.cold.6+0x13/0x4e
[  245.822125]  ? lapic_can_unplug_cpu.cold.29+0x37/0x37
[  245.822587]  nmi_trigger_cpumask_backtrace+0xde/0xe0
[  245.823039]  watchdog+0x223/0x2e0
[  245.823345]  ? hungtask_pm_notify+0x40/0x40
[  245.823730]  kthread+0x116/0x130
[  245.824028]  ? kthread_flush_work_fn+0x10/0x10
[  245.824436]  ret_from_fork+0x1f/0x40
[  245.824774] Sending NMI from CPU 2 to CPUs 0-1,3:
[  245.825215] NMI backtrace for cpu 0 skipped: idling at native_safe_halt+0xe/0x10
[  245.825217] NMI backtrace for cpu 1 skipped: idling at native_safe_halt+0xe/0x10
[  245.825228] NMI backtrace for cpu 3 skipped: idling at native_safe_halt+0xe/0x10
[  245.826209] Kernel panic - not syncing: hung_task: blocked tasks
[  245.829539] CPU: 2 PID: 42 Comm: khungtaskd Kdump: loaded Tainted: P           OE    --------- -  - 4.18.0-348.12.2.el8_5.x86_64 #1
[  245.831540] Hardware name: Red Hat Container-native virtualization/RHEL-AV, BIOS 1.14.0-1.module+el8.4.0+8855+a9e237a9 04/01/2014
[  245.833520] Call Trace:
[  245.834215]  dump_stack+0x5c/0x80
[  245.834979]  panic+0xe7/0x2a9
[  245.835707]  watchdog+0x22f/0x2e0
[  245.836450]  ? hungtask_pm_notify+0x40/0x40
[  245.837255]  kthread+0x116/0x130
[  245.837977]  ? kthread_flush_work_fn+0x10/0x10
[  245.838792]  ret_from_fork+0x1f/0x40
```
We can see, that `khungtaskd` found a process that was blocked for more than 120 seconds: 
```bash
[  245.808706] INFO: task systemd:1 blocked for more than 120 seconds.
```

Let's see backtrace of PID `1`:
```bash
crash> bt 1
PID: 1      TASK: ff47e40b01891e40  CPU: 2   COMMAND: "systemd"
 #0 [ff7ea32980327d88] __schedule at ffffffff8a77a2ad
 #1 [ff7ea32980327e18] schedule at ffffffff8a77a787
 #2 [ff7ea32980327e28] rwsem_down_read_slowpath at ffffffff8a77d320
 #3 [ff7ea32980327ec0] __do_page_fault at ffffffff89e75121
 #4 [ff7ea32980327f20] do_page_fault at ffffffff89e75267
 #5 [ff7ea32980327f50] page_fault at ffffffff8a80111e
    RIP: 00007f2411c4e27f  RSP: 00007fff58f62300  RFLAGS: 00010206
    RAX: 000055c2b6810bf0  RBX: 00007f2411f89bc0  RCX: 000055c2b67f67c0
    RDX: 00007f2411f89c50  RSI: 00007f2411f89c40  RDI: 00007f2411f89bc8
    RBP: 000000000000001d   R8: 000055c2b66515f0   R9: 000055c2b66515e0
    R10: 00007f2411f89bc0  R11: 0000000000000007  R12: 0000000000000003
    R13: 000000000000001d  R14: 00007f2411f89bc0  R15: 0000000000000030
    ORIG_RAX: ffffffffffffffff  CS: 0033  SS: 002b
```
There is an address where `__schedule` ended up `ffffffff8a77a2ad`, it should be simple `move`:
```bash
crash> dis ffffffff8a77a2ad -r | tail -n5
0xffffffff8a77a29f <__schedule+687>:	mov    %r13,%rsi
0xffffffff8a77a2a2 <__schedule+690>:	mov    %r12,%rdi
0xffffffff8a77a2a5 <__schedule+693>:	movzbl (%rax),%eax
0xffffffff8a77a2a8 <__schedule+696>:	callq  0xffffffff8a8001b0 <__switch_to_asm>
0xffffffff8a77a2ad <__schedule+701>:	mov    %rax,%rdi
```
There is nothing interesting, so I have no clue what actually happened here.
Also, due to lack of time I left this project as it is.

---
Author: Samuel Dobron (xdobro23)