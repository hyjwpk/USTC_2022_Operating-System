#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/err.h>

#include <linux/types.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS2021");
MODULE_DESCRIPTION("SYSFS_TEST!");
MODULE_VERSION("1.0");

//sysfs
#define SYSFS_TEST_RUN_STOP 0
#define SYSFS_TEST_RUN_START 1

// /sys/kerbel/mm/sysfs_test/pid
static unsigned int cycle = 0;
// /sys/kerbel/mm/sysfs_test/func
static unsigned int sysfs_test_func = 0;
//  /sys/kernel/mm/sysfs_test/sysfs_test_run
static unsigned int sysfs_test_run = SYSFS_TEST_RUN_STOP;
//  /sys/kernel/mm/sysfs_test/sleep_millisecs
static unsigned int sysfs_test_thread_sleep_millisecs = 20000;

static struct task_struct* sysfs_test_thread;

static DECLARE_WAIT_QUEUE_HEAD(sysfs_test_thread_wait);

static DEFINE_MUTEX(sysfs_test_thread_mutex);


static int sysfs_testd_should_run(void)
{
    return (sysfs_test_run & SYSFS_TEST_RUN_START);
}

static void print_hello(void)
{
    int i = 0;
    for (; i < cycle; i++)
        printk("hello world!\n");
}

static void print_hi(void)
{
    int i = 0;
    for (; i < cycle; i++)
        printk("hi world!\n");
}


static void sysfs_test_to_do(void)
{
    if (sysfs_test_func == 1)
        print_hello();
    else if (sysfs_test_func == 2)
        print_hi();
}

static int sysfs_testd_thread(void* nothing)
{
    set_freezable();
    set_user_nice(current, 5);
    while (!kthread_should_stop())
    {
        mutex_lock(&sysfs_test_thread_mutex);
        if (sysfs_testd_should_run())
            sysfs_test_to_do();
        mutex_unlock(&sysfs_test_thread_mutex);
        try_to_freeze();
        if (sysfs_testd_should_run())
        {
            schedule_timeout_interruptible(
                msecs_to_jiffies(sysfs_test_thread_sleep_millisecs));
        }
        else
        {
            wait_event_freezable(sysfs_test_thread_wait,
                sysfs_testd_should_run() || kthread_should_stop());
        }
    }
    return 0;
}


#ifdef CONFIG_SYSFS

/*
 * This all compiles without CONFIG_SYSFS, but is a waste of space.
 */

#define SYSFS_TEST_ATTR_RO(_name) \
        static struct kobj_attribute _name##_attr = __ATTR_RO(_name)

#define SYSFS_TEST_ATTR(_name)                         \
        static struct kobj_attribute _name##_attr = \
                __ATTR(_name, 0644, _name##_show, _name##_store)

static ssize_t sleep_millisecs_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", sysfs_test_thread_sleep_millisecs);
}

static ssize_t sleep_millisecs_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long msecs;
    int err;

    err = kstrtoul(buf, 10, &msecs);
    if (err || msecs > UINT_MAX)
        return -EINVAL;

    sysfs_test_thread_sleep_millisecs = msecs;

    return count;
}
SYSFS_TEST_ATTR(sleep_millisecs);

static ssize_t cycle_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", cycle);
}

static ssize_t cycle_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long tmp;
    int err;

    err = kstrtoul(buf, 10, &tmp);
    if (err || tmp > UINT_MAX)
        return -EINVAL;

    cycle = tmp;

    return count;
}
SYSFS_TEST_ATTR(cycle);


static ssize_t func_show(struct kobject* kobj,
    struct kobj_attribute* attr, char* buf)
{
    return sprintf(buf, "%u\n", sysfs_test_func);
}

static ssize_t func_store(struct kobject* kobj,
    struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    unsigned long tmp;
    int err;

    err = kstrtoul(buf, 10, &tmp);
    if (err || tmp > UINT_MAX)
        return -EINVAL;

    sysfs_test_func = tmp;

    return count;
}
SYSFS_TEST_ATTR(func);

static ssize_t run_show(struct kobject* kobj, struct kobj_attribute* attr,
    char* buf)
{
    return sprintf(buf, "%u\n", sysfs_test_run);
}

static ssize_t run_store(struct kobject* kobj, struct kobj_attribute* attr,
    const char* buf, size_t count)
{
    int err;
    unsigned long flags;
    err = kstrtoul(buf, 10, &flags);
    if (err || flags > UINT_MAX)
        return -EINVAL;
    if (flags > SYSFS_TEST_RUN_START)
        return -EINVAL;
    mutex_lock(&sysfs_test_thread_mutex);
    if (sysfs_test_run != flags)
    {
        sysfs_test_run = flags;
    }
    mutex_unlock(&sysfs_test_thread_mutex);

    if (flags & SYSFS_TEST_RUN_START)
        wake_up_interruptible(&sysfs_test_thread_wait);
    return count;
}
SYSFS_TEST_ATTR(run);



static struct attribute* sysfs_test_attrs[] = {
    // 扫描进程的扫描间隔 默认为20秒 
    &sleep_millisecs_attr.attr,
    &cycle_attr.attr,
    &func_attr.attr,
    &run_attr.attr,
    NULL,
};


static struct attribute_group sysfs_test_attr_group = {
    .attrs = sysfs_test_attrs,
    .name = "sysfs_test",
};
#endif /* CONFIG_SYSFS */


static int sysfs_test_init(void)
{
    int err;
    sysfs_test_thread = kthread_run(sysfs_testd_thread, NULL, "sysfs_test");
    if (IS_ERR(sysfs_test_thread))
    {
        pr_err("sysfs_test: creating kthread failed\n");
        err = PTR_ERR(sysfs_test_thread);
        goto out;
    }

#ifdef CONFIG_SYSFS
    err = sysfs_create_group(mm_kobj, &sysfs_test_attr_group);
    if (err)
    {
        pr_err("sysfs_test: register sysfs failed\n");
        kthread_stop(sysfs_test_thread);
        goto out;
    }
#else
    sysfs_test_run = KSCAN_RUN_STOP;
#endif  /* CONFIG_SYSFS */

out:
    return err;
}

static void sysfs_test_exit(void)
{
    if (sysfs_test_thread)
    {
        kthread_stop(sysfs_test_thread);
        sysfs_test_thread = NULL;
    }

#ifdef CONFIG_SYSFS

    sysfs_remove_group(mm_kobj, &sysfs_test_attr_group);

#endif

    printk("sysfs_test exit success!\n");
}

/* --- 随内核启动  ---  */
// subsys_initcall(kscan_init);
module_init(sysfs_test_init);
module_exit(sysfs_test_exit);