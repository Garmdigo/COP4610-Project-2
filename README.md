COP4610-Project-2 <br/>
Diego Gutierrez, Samuel Parmer, Christian Gazmuri <br/>
Instructions:<br/>
Part 1: <br/>
To compile both Syscalls.c and empty.c, type "make" in the part 1 folder. Syscall.c adds 6 system calls while empty.c is an empty C program. <br/>
Once compiled use, strace -o log1.txt ./Syscall.x and strace -o log2.txt ./empty.x. After use cat log1 and log2 to see what syscalls are used. <br/>
Part 2: To compile, type "make" in the part 2 folder. <br/>
Type "sudo insmod my_xtime.ko" to install the module. issue sudo rmmod mytime to remove the module. dmesg | tail or proc file can be used to see results. Before removing, use cat /proc/timed and then sleep to see the time that has elapse. <br/>
Part 3: issue "sudo make" in syscallModule directory to compile. Use sudo insmod syscallModule.ko to install the kernel module. Test by running consumer.x and producer.x. remove module with sudo rmmod syscallModule.ko <br/>
Division of labor:<br/>
Diego: part 1, my_xtime, documentation, testing.<br/>
Samuel:my_xtime, system calls, elevator module, schedulng algorithm  <br/>
Christian : contributed on parts 2 and 3, testing <br/>

Unfinished portionis and bugs, other notes:

number of serviced requests per floor sometimes seems inconsistant after issuing several requests.

Proc system works but is not being used. All diagnostic prints go to kernel log as is.
Weight system is not implemeted.
<br/>
