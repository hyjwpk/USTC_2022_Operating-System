// 必备头函数
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

// 该模块的LICENSE
MODULE_LICENSE("GPL");
// 该模块的作者
MODULE_AUTHOR("OS2021");
// 该模块的说明
MODULE_DESCRIPTION("This is a simple example!/n");

// 该模块需要传递的参数
static int loop = -1;
module_param(loop, int, 0644);


// 初始化入口
// 模块安装时执行
// 这里的__init 同样是宏定义，主要的目的在于
// 告诉内核，加载该模块之后，可以回收init.text的区间
static int __init print_hello_init(void)
{
    int i = 0;
    // 输出信息，类似于printf()
    // printk适用于内核模块
    printk(KERN_ALERT" module init!\n");
    while(i < loop)
    {
        printk(KERN_ALERT" hello world!\n");
        i++;
    }
    return 0;
}

// 模块卸载时执行
// 同上
static void __exit print_hello_exit(void)
{
    printk(KERN_ALERT" module has exited!\n");
}

// 模块初始化宏，用于加载该模块
module_init(print_hello_init);
// 模块卸载宏，用于卸载该模块
module_exit(print_hello_exit);
