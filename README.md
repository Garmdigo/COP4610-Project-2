COP4610-Project-2 <br/>
Diego Gutierrez, Samuel Parmer, Christian Gazmuri <br/>
Instructions:<br/>
Part 1: <br/>
To compile both Syscalls.c and empty.c, type "make" in the part 1 folder. Syscall.c adds 6 system calls while empty.c is an empty C program. <br/>
Once compiled use, strace -o log1.txt ./Syscalls.x and strace -o log2.txt ./empty.x. After use cat log1.txt and log2.txt to see what syscalls are used and how many were added. <br/>
Part 2: To compile, type "make" in the part 2 folder. <br/>
Type "sudo insmod my_xtime.ko" to install the module. issue sudo rmmod mytime to remove the module. dmesg | tail or proc file can be used to see results. Before removing, use cat /proc/timed and then sleep to see the time that has elapse. <br/>
Part 3: issue "sudo make" in syscallModule directory to compile. Use sudo insmod syscallModule.ko to install the kernel module. Test by running consumer.x and producer.x. remove module with sudo rmmod syscallModule.ko <br/>
Division of labor:<br/>
Diego: part 1, my_xtime, documentation, testing.<br/>
Samuel:my_xtime, system calls, elevator module, schedulng algorithm  <br/>
Christian : contributed on parts 2 and 3, testing <br/>

Unfinished portionis and bugs, other notes:

proc does not accurately show what is going on when the elevator moves, passengers picked up etc. correct info is available by using dmesg. 
Weight system may not be correct.
<br/>
