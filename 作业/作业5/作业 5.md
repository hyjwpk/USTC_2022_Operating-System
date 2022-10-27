# 作业五

## 1. Consider a RAID organization comprising five disks in total, how many blocks are accessed in order to perform the following operations for RAID-5 and RAID-6?

### a. An update of one block of data

​	如果采用RMW

​		RAID-5: 两个块，分别是数据块和所在条的校验块

​		RAID-6: 三个块，分别是数据块和所在条的两个校验块

​	如果采用RRW

​		RAID-5: 五个块，是写入的数据块所在条的所有块，包括四个数据块和一个校验块

​		RAID-6: 五个块，是写入的数据块所在条的所有块，包括三个数据块和两个校验块

### b. An update of seven continuous blocks of data. Assume that the seven contiguous blocks begin at a boundary of a stripe.

​	如果采用RMW

​		RAID-5: 九个块，七个数据块和数据块所在的两个条的校验块（每条一个）

​		RAID-6: 十三个块，七个数据块和数据块所在的三个条的校验块（每条两个）

​	如果采用RRW

​		RAID-5: 十个块，是写入的数据块所在的两个条的数据块和校验块

​		RAID-6: 十五个块，是写入的数据块所在的三个条的数据块和校验块

## 2. Suppose that a disk drive has 5,000 cylinders, numbered 0 to 4999. The drive is currently serving a request at cylinder 2150, and the previous request was at cylinder 1805. The queue of pending requests, in FIFO order, is: 2069, 1212, 2296, 2800, 544, 1618, 356, 1523, 4965, 3681 Starting from the current head position, what is the total distance (in cylinders) that the disk armmoves to satisfy all the pending requests for each of the following disk-scheduling algorithms?

### a. FCFS

访问顺序： 2069, 1212, 2296, 2800, 544, 1618, 356, 1523, 4965, 3681

移动距离：13011

### b. SSTF

访问顺序： 2069, 2296, 2800, 3681, 4965, 1618, 1523, 1212, 544, 356

移动距离：7586

### c. SCAN

访问顺序： 2296, 2800, 3681, 4965, 2069, 1618, 1523, 1212, 544, 356

移动距离：7492

### d. LOOK

访问顺序： 2296, 2800, 3681, 4965, 2069, 1618, 1523, 1212, 544, 356

移动距离：7424

### e. C-SCAN

访问顺序： 2296, 2800, 3681, 4965, 356, 544, 1212, 1523, 1618, 2069

移动距离：9917

### f. C-LOOK

访问顺序： 2296, 2800, 3681, 4965, 356, 544, 1212, 1523, 1618, 2069

移动距离：9137

## 3. Explain what open-file table is and why we need it.

​	在执行open()系统调用后，操作系统会把打开的文件信息添加到open-file table中，在执行close()系统调用后，操作系统会把删除的文件信息从open-file table中删除。open-file table用于保存当前打开的所有文件信息。

​	在有了open-file table后，在对文件执行一系列操作前先使用open()，这样文件的信息添加到open-file table中，之后再执行操作时就可以直接定位文件，而不用每次进行操作时都去查找文件

## 4. What does “755” mean for file permission?

7 5 5 对应的二进制形式是111 101 101，表明文件所有者对文件具有读、写、执行的权限，组成员和其他用户只具有读和执行的权限

## 5. Explain the problems of using continuous allocation for file system layout and how to solve them.

问题：会产生外部碎片

解决方案：

​	1.碎片整理，带来额外的开销

​	2.将大文件分成多个部分存入固定大小的块中，在每个块中添加下一块的索引以构成链表

问题：文件大小无法增长

解决方案：

​	1.移动文件前后的数据，或者将文件复制到一个较大的空闲位置，带来额外的开销

​	2.将大文件分成多个部分存入固定大小的块中，在分块存储后，文件大小增长只需将末尾块指向新分配的空闲块

## 6. What are the advantages of the variation of linked allocation that uses a FAT to chain together the blocks of a file? What is the major problem of FAT?

​	将每个块的下一个块的信息集中在一起用一个数组存储，这样在需要随机访问某个文件的中间一块时不需要顺序读取前面的所有块，只需依次访问FAT表，改善了随机访问时间，同时由于FAT表是一个集中在一起的数组，可以将整个FAT表读入内存后再访问以加快访问速度。

​	FAT的主要问题是需要将整个表读入内存中访问，这将带来额外的I/O和内存空间开销，用空间交换速度，若不将FAT表缓存到内存中，每次访问都需要先读FAT表，再访问数据，带来较大的寻道时间。

## 7. Consider a file system similar to the one used by UNIX with indexed allocation, and assume that every file uses only one block. How many disk I/O operations might be required to read the contents of a small local file at */a/b/c* in the following two cases? Should provide the detailed workflow.

### a. Assume that none of the disk blocks and inodes is currently being cached.

1.读根目录

2.读/a对应的inodes

3.读/a对应的disk blocks

4.读/a/b对应的inodes

5.读/a/b对应的disk blocks

6.读/a/b/c对应的inodes

7.读/a/b/c对应的disk blocks

总共七次I/O操作

### b. Assume that none of the disk blocks is currently being cached but all inodes are in memory.

1.读根目录

2.读/a对应的inodes(已缓存)

3.读/a对应的disk blocks

4.读/a/b对应的inodes(已缓存)

5.读/a/b对应的disk blocks

6.读/a/b/c对应的inodes(已缓存)

7.读/a/b/c对应的disk blocks

总共四次I/O操作

## 8. Consider a file system that uses inodes to represent files. Disk blocks are 8-KB in size and a pointer to a disk block requires 4 bytes. This file system has 12 direct disk blocks, plus single, double, and triple indirect disk blocks. What is the maximum size of a file that can be stored in this file system?

​	一个文件的索引节点能指向的数据块大小总和为：$12*2^x+2^{2x-2}+2^{3x-4}+2^{4x-6} \approx 64TB$。文件大小在索引节点中占64bit，能表示的最大文件大小大于64TB，因此文件最大大小约为64TB。

## 9. What is the difference between hard link and symbolic link?

​	硬链接是指向一个已存在的文件的入口，指向已存在的索引节点，不会创建新的文件。硬链接会增加文件的链接计数，硬链接是给文件增加了一个新的名字。当删除硬链接时，文件的链接计数减1，当减为0时文件被删除

​	符号链接会创建一个新的文件，它存储指向的路径地址。创建符号链接并不改变文件的链接计数。删除符号链接指向的文件，符号链接依然存在，但无法访问指向的文件。

## 10. What is the difference between data journaling and metadata journaling? Explain the operation sequence for each of the two journaling methods.

数据日志：

​	第一步：在日志中写入 TxB, I[v2], B[v2],Db，可以并行

​	第二步：在日志中写入TxE

​	第三步：将元数据和数据写入磁盘对应位置

​	第四步：删除日志内容

元数据日志：

​	第一步：在日志中写入 TxB, I[v2], B[v2] ，同时将数据写入磁盘对应位置

​	第二步：在日志中写入TxE

​	第三步：将元数据写入磁盘对应位置

​	第四步：删除日志内容

数据日志和元数据日志的区别在于对数据的处理，数据日志先将数据写入日志，之后再写入磁盘；元数据日志在将元数据写入日志的同时并行将数据写入磁盘，这样减少了一次对日志的写入。

## 11. What are the three I/O control methods?

中断：由I/O设备触发CPU中断请求

轮询：CPU不断读取I/O设备的busy bit直到I/O空闲

DMA：将DMA指令块位置写入DMA控制器，DMA控制器独立于CPU之外完成I/O请求

## 12. What services are provided by the kernel I/O subsystem?

I/O调度

- 控制一个单设备队列
- 重排序I/O请求
- 平均等待时间、公平性等

设备间缓冲

- 处理设备间速度不匹配
- 处理设备间传输大小不匹配
- 控制复制时的语义问题

缓存

- 数据的复制

- 提升性能的关键
- 有时与缓冲结合

假脱机技术(Spooling)

- 当设备一次只能处理一个请求时

错误处理和I/O保护

- 操作系统可以从磁盘读错误、磁盘不可用、瞬时写入失败中恢复
- 所有的I/O指令被定义为特权的

电源管理等
