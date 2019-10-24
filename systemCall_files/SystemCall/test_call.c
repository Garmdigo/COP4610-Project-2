#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

// this one sets up the STUB

/* System call stub 0 -- original */
long (*STUB_start_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_start_elevator);

/* System call wrapper */
SYSCALL_DEFINE0(start_elevator) {
	printk(KERN_NOTICE "Inside SYSCALL_DEFINE0 335 block. %s: \n", __FUNCTION__);
	if (STUB_start_elevator != NULL)
		return STUB_start_elevator();
	else
		return -ENOSYS;
}

/* System call stub 1 */
long (*STUB_issue_request)(int, int, int) = NULL;
EXPORT_SYMBOL(STUB_issue_request);

/* System call wrapper */
SYSCALL_DEFINE3(issue_request, int, test_int1, int, test_int2, int, test_int3) {
    printk(KERN_NOTICE "Inside SYSCALL_DEFINE3 336 block. %s: Your params: %d, %d, %d\n", __FUNCTION__, test_int1, test_int2, test_int3);
    if (STUB_issue_request != NULL)
        return STUB_issue_request(test_int1, test_int2, test_int3);
    else
        return -ENOSYS;
}

/* System call stub 2 */
long (*STUB_stop_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_stop_elevator);

/* System call wrapper */
SYSCALL_DEFINE0(stop_elevator) {
    printk(KERN_NOTICE "Inside SYSCALL_DEFINE0 337 block. %s: \n", __FUNCTION__);
    if (STUB_stop_elevator != NULL)
        return STUB_stop_elevator();
    else
        return -ENOSYS;
}
