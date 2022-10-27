# 操作系统作业二

## 1. Including the initial parent process, how many processes are created by the program shown in Figure 1?

​	每当循环内的`fork`执行一次，总的进程数变成原来的`两倍`，循环会执行`四次`，则包括初始的进程，总共有$2^4=16$个进程被创建。

## 2. Explain the circumstances under which the line of code marked printf ("LINE J) in Figure 2 will be reached

​	要到达`printf ("LINE J)`所在的条件分支，需要`pid==0`，只有在`fork`创建的子进程中会进入此条件分支，但`execlp`函数会用新的程序覆盖当前进程的地址空间，在`execlp`执行成功的条件下，子进程不会执行`printf ("LINE J)`，只有当`fork`成功但`execlp`失败返回的情况下会执行`printf ("LINE J)`。

## 3.Using the program in Figure 3, identify the values of pid at lines A, B, C, and D. (Assume that the actual pids of the parent and child are 2600 and 2603, respectively.)

- 在A行，A行只有子进程会执行，`pid`的值为`fork`在子进程中的返回值0
- 在B行，B行只有子进程会执行，`pid1`的值为子进程的真实`pid` 2603
- 在C行，C行只有父进程会执行，`pid`的值为`fork`在父进程中的返回值，即为子进程的`pid` 2603
- 在D行，D行只有父进程会执行，`pid1`的值为父进程的真实`pid` 2600

## 4.Using the program shown in Figure 4, explain what the output will be at lines X and Y.

- X行的输出为 `0 -1 -4 -9 -16 `
- Y行的输出为 `0 1 2 3 4 ` 

##  5. For the program in Figure 5, will LINE X be executed, and explain why.

​	当`execl`执行成功时，X行不会被执行，因为`execl`函数会用新的程序覆盖当前进程的地址空间。但当`execl`执行失败时，`execl`会返回，X行会被执行。

## 6. Explain why “terminated state” is necessary for processes.

​	进程终止后，进程的状态变成`终止状态`，此时子进程可以向父进程发送`SIGCHILD`信号，直到父进程调用`wait`函数后，`wait`函数返回子进程的退出状态与子进程的标识符，此时子进程被真正删除。`终止状态`的存在可以便于父进程获知子进程的退出状态以及哪个子进程已经退出。

## 7. Explain what a zombie process is and when a zombie process will be eliminated (i.e., its PCB entry is removed from kernel).

​	`僵尸进程`是指，在进程终止后，它的资源已经被操作系统释放，但在进程表中仍保有它的条目，它会向父进程发送`SIGCHILD`信号，直到父进程调用`wait`函数后，它的`PCB`入口被内核销毁，当父进程未调用`wait`函数便终止时，`init`会成为`僵尸进程`的父进程，`init`会定时调用`wait`以便删除僵尸进程

## 8. Explain what data will be stored in user-space and kernel-space memory for a process.

- 进程在`用户空间`会存储`全局变量`、局部变量、`动态申请内存`、`常量`、`代码段`
- 进程在`内核空间`会存储`PCB`

## 9. Explain the key differences between exec() system call and normal function call.

- `exec`执行后会用新的程序覆盖当前进程的地址空间，从而继续执行新的程序，在`exec`执行成功时，不会再返回原程序，只有当`exec`失败时会返回原程序继续执行
- 一般的函数调用执行后，无论成功与否都会返回原程序继续执行

## 10.What are the benefits of multi-threading? Which of the following components of program state are shared across threads in a multithreaded process? a. Register values b. Heap memory c. Global variables d. Stack memory

`多线程`的优点：一个应用程序可能会被要求执行多个类似任务，在`单线程`的运行模式下，每次只能处理一个任务，效率较低。可以通过创建多个进程来解决这一问题，但每个进程都会有自己的`独立内存空间`，对重复任务而言，有大量的数据是可以共享的，创建多个进程会浪费资源与时间。在`多线程`下，每个线程可以执行各自的任务，共享资源，与创建进程相比，创建线程更加快速，在多处理器的体系结构下，多线程可以实现并行执行，提高效率。

下列选项中可以被多线程共享的是`b、堆空间`	`c.全局变量`

## 11.Consider the following code segment: a. How many unique processes are created? b. How many unique threads are created?

包含初始进程在内，会创建`6`个进程

会创建`2`个线程（不包括每个进程的主线程）

## 12.The program shown in the following figure uses Pthreads. What would be the output from the program at LINE C and LINE P?

C行输出5，P行输出0

## 13.What are the differences between ordinary pipe and named pipe?

- 普通管道在父进程和子进程之间单向通信，在`UNIX`系统上，使用`pipe`函数创建管道，通过`read`和`write`函数读写，在父子进程结束后，普通管道会被销毁
- 命名管道可以双向通信，并且进程间的父子关系不是必须的，在创建命名管道后，多对进程都可以用它通信。`UNIX`系统的命名管道是半双工的，`Windows`系统的命名管道支持全双工，在`UNIX`系统上，管道命名为`FIFO`，使用`mkfifo`创建，`open`、`read`、`write`、`close`进行操作，命名管道在被销毁前可以一直存在，即使使用它通信的进程已经结束
