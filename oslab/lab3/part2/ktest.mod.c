#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd9726f80, "module_layout" },
	{ 0xe5896e3e, "sysfs_remove_group" },
	{ 0x1e20e5b6, "filp_close" },
	{ 0x514011c4, "kthread_stop" },
	{ 0x13a7dc87, "sysfs_create_group" },
	{ 0x4239a8b9, "mm_kobj" },
	{ 0x3df6b5ab, "wake_up_process" },
	{ 0x40794748, "kthread_create_on_node" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0x50d4de4, "pv_ops" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0xd304cfde, "boot_cpu_data" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0x974db752, "kernel_write" },
	{ 0x69acdf38, "memcpy" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0xf7e419f3, "mmput" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xa916b694, "strnlen" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xe6a83662, "get_task_mm" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x4482cdb, "__refrigerator" },
	{ 0xe3f25314, "freezing_slow_path" },
	{ 0x1000e51, "schedule" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x54496b4, "schedule_timeout_interruptible" },
	{ 0x7f02188f, "__msecs_to_jiffies" },
	{ 0x7ab88a45, "system_freezing_cnt" },
	{ 0xf7cbd1dd, "filp_open" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xabbb39ec, "set_user_nice" },
	{ 0x9e61bb05, "set_freezable" },
	{ 0x69af1880, "current_task" },
	{ 0xc8a427a8, "pid_task" },
	{ 0x18d8f136, "find_get_pid" },
	{ 0x800473f, "__cond_resched" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xc5850110, "printk" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x5c3c7387, "kstrtoull" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B819EA3BE002C52F17AE455");
