COP4610-Project-2 <br/>
Diego Gutierrez, Samuel Parmer & Christian Gazmuri <br/>
Instructions:<br/>
Part 1: <br/>
To compile both Syscalls.c and empty.c, type "make" in the part 1 folder. Syscall.c adds 6 system calls while empty.c is an empty C program. <br/>
Once compiled use, strace -o log1.txt Syscall.c and strace -o log2.txt empty.c to view the system calls info. <br/>
Part 2: To compile, type "make" in the part 2 folder. <br/>
Type "sudo insmod my_xtime.ko" to install the module, dmesg | tail, cat /proc/mytimed ,  sudo rmmod mytime(to remove the module) and  dmesg | tail. Before removing, use cat /proc/timed and then sleep to see the time that has elapse. <br/>
Part 3:<br/>
Division of labor:<br/>
Diego: part 1, my_xtime, documentation.<br/>
Samuel:system calls, elevator module, scheduler  <br/>
Christian :  <br/>
Known bugs: <br/>
