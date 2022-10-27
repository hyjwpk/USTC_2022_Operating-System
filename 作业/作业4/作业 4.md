# 操作系统作业四

## 1. Explain the following terms：

​	segmentation fault：当试图非法访问一段并未被允许访问的空间时，操作系统会提示错误，例如访问空指针，读写未分配的段，写入只读段都会产生段错误。

​	TLB：Translation lookaside buffer是一块用于将逻辑地址转换为物理地址的高速缓存，它缓存了部分页号和帧号的对应关系，在查询页表前，会先查找TLB，若对应的项存在TLB中，可直接得到帧号，否则再去查找页表，TLB的查找速度比页表快，在TLB找到时，可以加速转换过程。

​	Page fault：页错误在CPU的内存管理单元试图去访问一个页时发现这个页不在内存中，即访问的页无效时产生。

​	Demand paging：请求调页指CPU仅在需要时才加载页面的技术，常用于虚拟内存系统，例如申请内存后，操作系统并不会真正分配内存，而是分配虚拟内存，等到对内存访问时才真正分配物理内存。在试图对虚拟内存访问时会产生页错误，然后物理内存会被分配给引起页错误的虚拟内存。

## 2. Introduce the concept of thrashing, and explain under what circumstance thrashing will happen.

​	如果一个进程被分配的帧不够，就会频繁产生页错误，然后引起页面调度，这种高度的页面调度活动称为抖动。

​	当一个进程没有足够的帧时，当进程需要访问一个不在内存中的页面就会引起页错误，此时需要置换其它的页面，但其它的页面很快也会被再次使用，于是进程会快速再次产生页错误，如此不断快速进行，就会频繁产生页错误。

## 3.Consider a paging system with the page table stored in memory.

### a. If a memory reference takes 50 nanoseconds, how long does a paged memory reference take?

​	需要先化50ns查找页表将虚拟地址转换为物理地址，再花50ns访问物理页面，共计100ns

### b. If we add TLBs, and 75 percent of all page-table references are found in the TLBs, what is the effective memory reference time? (Assume that finding a page-table entry in the TLBs takes 2 nanoseconds, if the entry is present.)

当TLB命中时，需要花费2ns得到物理地址，

若当TLB未命中时需要花费52ns得到物理地址（先查找TLB花费2ns，未命中后再查页表需要50ns），平均需要$2*75\%+52*25\%=14.5ns$得到物理地址，则平均一共需要64.5ns

若当TLB未命中时只需要花费50ns得到物理地址（先查找TLB未命中不花费时间，未命中后再查页表需要50ns），则平均需要$2*75\%+52*25\%=14ns$，则平均一共需要64ns

## 4.Assume a program has just referenced an address in virtual memory. Describe a scenario how each of the following can occur: (If a scenario cannot occur, explain why.)

### TLB miss with no page fault

​	需要的页面在内存中，但物理页面和虚拟地址的对应关系却不在TLB中，可能是由于在上次访问该页面之后在TLB中被其它页面与虚拟地址的对应关系置换。

### TLB miss and page fault

​	需要的页面不在内存中，在TLB中也未缓存物理页面和虚拟地址的对应关系，可能是因为之前从未访问过，或上次访问后已被其它页面从TLB及内存中置换

### TLB hit and no page fault

​	需要的页面在内存中，且物理页面和虚拟地址的对应关系在TLB中缓存，可能是近期访问过该页面

### TLB hit and page fault

​	需要的页面不在内存中，物理页面和虚拟地址的对应关系却在TLB中缓存，一般不可能发生此情况，因为TLB的大小远小于内存中的页面数，若需要的页面在内存中已经被置换，在TLB中的缓存应该已经被替换

##  5. Assume we have a demand-paged memory. The page table is held in registers. It takes 8 milliseconds to service a page fault if an empty page is available or the replaced page is not modified, and 20 milliseconds if the replaced page is modified. Memory access time is 100 nanoseconds. Assume that the page to be replaced is modified 70 percent of the time. What is the maximum acceptable page-fault rate for an effective access time of no more than 200 nanoseconds?

​	假设页错误发生的速度（概率）为x，则x应满足
$$
x*(70\%*20000+30\%*8000)<200-100
$$
​	解得$x<6.098*10^{-3}$

## 6. Consider the following page reference string: 7, 2, 3, 1, 2, 5, 3, 4, 6, 7, 7, 1, 0, 5, 4, 6, 2, 3, 0, 1.Assuming demand paging with three frames, how many page faults would occur for the following replacement algorithms?

​	以下表格的前三行表示每次页面访问后内存中的页面，第四行为F表示本次访问产生Page fault，S表示没有产生Page fault

### LRU replacement

| 7    | 7    | 7    | 1    | 1    | 1    | 3    | 3    | 3    | 7    | 7    | 7    | 7    | 5    | 5    | 5    | 2    | 2    | 2    | 1    |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
|      | 2    | 2    | 2    | 2    | 2    | 2    | 4    | 4    | 4    | 4    | 1    | 1    | 1    | 4    | 4    | 4    | 3    | 3    | 3    |
|      |      | 3    | 3    | 3    | 5    | 5    | 5    | 6    | 6    | 6    | 6    | 0    | 0    | 0    | 6    | 6    | 6    | 0    | 0    |
| F    | F    | F    | F    | S    | F    | F    | F    | F    | F    | S    | F    | F    | F    | F    | F    | F    | F    | F    | F    |

18次

###  FIFO replacement

| 7    | 7    | 7    | 1    | 1    | 1    | 1    | 1    | 6    | 6    | 6    | 6    | 0    | 0    | 0    | 6    | 6    | 6    | 0    | 0    |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
|      | 2    | 2    | 2    | 2    | 5    | 5    | 5    | 5    | 7    | 7    | 7    | 7    | 5    | 5    | 5    | 2    | 2    | 2    | 1    |
|      |      | 3    | 3    | 3    | 3    | 3    | 4    | 4    | 4    | 4    | 1    | 1    | 1    | 4    | 4    | 4    | 3    | 3    | 3    |
| F    | F    | F    | F    | S    | F    | S    | F    | F    | F    | S    | F    | F    | F    | F    | F    | F    | F    | F    | F    |

17次

### Optimal replacement

7, 2, 3, 1, 2, 5, 3, 4, 6, 7, 7, 1, 0, 5, 4, 6, 2, 3, 0, 1

| 7    | 7    | 7    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    | 1    |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
|      | 2    | 2    | 2    | 2    | 5    | 5    | 5    | 5    | 5    | 5    | 5    | 5    | 5    | 4    | 6    | 2    | 3    | 3    | 3    |
|      |      | 3    | 3    | 3    | 3    | 3    | 4    | 6    | 7    | 7    | 7    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    |
| F    | F    | F    | F    | S    | F    | S    | F    | F    | F    | S    | S    | F    | S    | F    | F    | F    | F    | S    | S    |

13次

## 7. Explain what Belady’s anomaly is, and what is the feature of stack algorithms which never exhibit Belady’s anomaly?

​	Belady’s anomaly是指对某些页面置换算法，随着分配帧数量的增加，缺页错误率可能会增加

​	stack algorithms的特点是当分配帧为n时在内存中的页面，当n变为n+1时这些页面也会在内存中
