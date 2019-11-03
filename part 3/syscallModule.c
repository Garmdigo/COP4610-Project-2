#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int, int, int);
extern long (*STUB_stop_elevator)(void);

#define ADULT 0
#define CHILD 1
#define ROOM_SERVICE 2
#define BELLHOP 3
#define DEFINE_SPINLOCK(x) spinlock_t x = __SPIN_LOCK_UNLOCKED(x)

enum eventType {
PASSENGER,
ELEVATOR,
PROC,
STOP
};

// module does not require recompile of kernel.

struct list_head floors[9];

typedef struct Elevator {
	struct list_head list;
	int totalWeight;
	int totalPassengers;
	int currentFloor;
	int destinationFloor;
	char *status;
} Elevator;

bool elevatorStarted = false;

void initElevator(Elevator e){
//e->list = NULL; HOW DO I INIT THIS LIST?
//maybe e.list = LIST_HEAD_INIT(e.list) for static call of list head
// or e.list = INIT_LIST_HEAD(e.list) for dynamic call of list head
e.status = "ONLINE";
e.totalWeight = 0;
e.totalPassengers = 0;
e.currentFloor = 1;
}


DECLARE_WAIT_QUEUE_HEAD(wq);

// list events to be processed by kernel thread
struct list_head events_list;
DEFINE_SPINLOCK(events_lock);


// structure describing the event to be processed
typedef struct event {
    struct list_head lh;
    int eventType;
    void *data;		//Assign void pointer to correct struct for event type
} Event;

typedef struct passenger {
int initialFloor;
int destinationFloor;
int type;
int passengerCount;
int weight;
char *name;
} Passenger;

void shutdownElevator(void){
elevatorStarted = false;
printk("elevator going offline");
}


int eventHandler(Event *ev){

	Passenger *p = (Passenger *)ev->data;

	switch(ev-> eventType){



		case PASSENGER:
		//Assign void pointer to passenger struct
		

 	switch (p->type) { //Switch on passenger type
		case ADULT:
			p->passengerCount = 1;
			p->weight = 2;
			p->name = "adult";
			break;
		case CHILD:
			p->passengerCount = 1;
			p->weight = 1;
			p->name = "child";
			break;
		case ROOM_SERVICE:
			p->passengerCount = 2;
			p->weight = 4;
			p->name = "room_service";
			break;

		case BELLHOP:
			p->passengerCount = 2;
			p->weight = 8;
			p->name = "bellhop";
			break;		

		default:
			return 0;
	}


			break;

 
		case ELEVATOR:
			int maxWeight = 15;
			int maxPassengers = 10;
			
			// if(e.totalWeight+(p->weight) > maxWeight){
			//	begin moving towards passenger destinations }
			// else if(e.totalPassengers+(p->passengerCount) > maxPassengers){
			// 	begin moving towards passenger destinations }

			break;
		case PROC:

			break;

		case STOP:
		return 1;
			break;		

		default:
			return 0;
}

//ev->initialFloor




}


Event* get_next_event(void)
{
	Event *e;
	spin_lock(&events_lock);
	e = list_first_entry(&events_list, Event, lh);
	if(e) //should there be braces here?
	list_del(&events_list);
	spin_unlock(&events_lock);

	return e;
}

void send_event(Event *ev)
{
	spin_lock(&events_lock);
    list_add(&ev->lh, &events_list);
    spin_unlock(&events_lock);
    wake_up(&wq);
}



int my_thread_f(void *data)  //Function that is passed into kthread_run 
{
        Event *e;
	Elevator elevator;
	initElevator(elevator);
    
    
	while (true) {
	wait_event(wq, (e = get_next_event()));
	
	/* Event processing */

	if (eventHandler(e) == 1)
		break;
	}
	
	shutdownElevator();
	do_exit(0);
}


long my_start_call(void) {
	printk(KERN_NOTICE "%s: \n", __FUNCTION__);
	
	if(!elevatorStarted){
	//add boolean to keep track of if elevator is online or offline
	/* start and start kthread */
	printk("Elevator has been started from module");	
	kthread_run(my_thread_f, NULL, "%s-kthread%d", "elevator", 0); //infinite loop, breaks when stop is TRUE
	elevatorStarted = true;
	return 0;
	} else {
	return 1;
	}
	printk("Elevator ");
	return 0;
}
long my_request_call(int p1, int p2, int p3) {
    printk(KERN_NOTICE "%s: Your int is %d,  %d, %d\n", __FUNCTION__, p1, p2, p3);
        Event *e = (Event *)kmalloc(sizeof(Event), __GFP_RECLAIM);
	
	Passenger *p = (Passenger *)kmalloc(sizeof(Passenger), __GFP_RECLAIM);

	p->type = p1;
	p->initialFloor = p2;
	p->destinationFloor = p3;
	
	e->eventType = PASSENGER;
	e-> data = (void *)p;

	send_event(e);
    return 336;
}

long my_stop_call(void) {
    printk(KERN_NOTICE "%s: \n", __FUNCTION__);

	printk("Stopping elevator");

	Event *e = kmalloc(sizeof(Event), __GFP_RECLAIM);	
	e -> eventType = STOP;

	send_event(e);
    return 337;
}


// puts my_test_call into STUB
static int hello_init(void) {
	STUB_start_elevator = my_start_call;
    STUB_issue_request = my_request_call;
    STUB_stop_elevator = my_stop_call;

	//set bool stop to true, send event to stop
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

