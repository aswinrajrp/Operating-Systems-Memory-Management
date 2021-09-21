# Operating-Systems-Memory-Management

The OS provides virtual memory to processes which allows them to each have an isolated address space and store only what it needs in physical memory. But when physical memory is not enough to store all the processes’ working sets, thrashing happens and the system performance suffers greatly. Unfortunately, Linux leaves this problem unaddressed. In this project, we implementeed a new feature to help Linux avoid thrashing!

We implemented a kernel module to estimate the working set size (WSS) of a given process. Eventually, Linux can use this information to make sure that it does not oversubscribe the physical memory and thereby avoid thrashing. In this project, we focused on implementing the feature of estimating a process’ WSS.
