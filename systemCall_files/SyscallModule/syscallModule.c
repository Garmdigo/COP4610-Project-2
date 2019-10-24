#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
MODULE_LICENSE("GPL");

// module does not require recompile of kernel.

extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int, int, int);
extern long (*STUB_stop_elevator)(void);

long my_start_call(void) {
	printk(KERN_NOTICE "%s: \n", __FUNCTION__);
	printk("Elevator started");
	return 335;
}
long my_request_call(int p1, int p2, int p3) {
    printk(KERN_NOTICE "%s: Your int is %d,  %d, %d\n", __FUNCTION__, p1, p2, p3);
    return 336;
}
long my_stop_call(void) {
    printk(KERN_NOTICE "%s: \n", __FUNCTION__);
    return 337;
}


// puts my_test_call into STUB
static int hello_init(void) {
	STUB_start_elevator = my_start_call;
    STUB_issue_request = my_request_call;
    STUB_stop_elevator = my_stop_call;
	return 0;
}


// puts null into the STUBS
static void hello_exit(void) {
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;
}

module_init(hello_init);
module_exit(hello_exit);

