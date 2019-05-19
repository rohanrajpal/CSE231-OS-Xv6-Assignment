Assignment was done in a group by
- Savit Gupta
- Rohan Rajpal
- Madhav Sainanee

You can try out the assignment(mentioned below) yourself. Our solution to the assignment is this repository

To view the complete assignment click
[here](https://drive.google.com/file/d/1Qpd8eORYzfsM6rSfShBYqLMkfwcvAfWT/view?usp=sharing)

# Demand paging in xv6
The goal of this assignment to understand the paging subsystem of an operating system. The first component you need to implement is demand paging.
Demand paging lazily maps a physical page corresponding to a virtual page
during the time of dereferencing. In addition to demand paging, you also need
to implement swapping. If the system runs out of physical memory, then the
swapping subsystem reuse an already used physical page by saving its contents
to secondary storage (e.g., disk).

# Environment
For this assignment, you need to install qemu.  
Run  
`sudo apt-get install qemu`  
Please clone the assignment repo from https://github.com/Systems-IIITD/  
xv6-paging.git.  
Run  
`make qemu`  
It will boot the xv6 OS in qemu emulator. You can also redirect the output to  
a file using, make qemu > log.txt.  
The default process is the shell. Type memtest1 to run memtest1.  
Currently ( Ours does not :) ), it panics because the page fault handler is not implemented. After,
you have correctly implemented everything, memtest1 and memtest2 should run
normally
