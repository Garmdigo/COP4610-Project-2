#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Elevator Module");

extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int, int, int);
extern long (*STUB_stop_elevator)(void);

#define ADULT 0
#define CHILD 1
#define ROOM_SERVICE 2
#define BELLHOP 3

#define ENTRY_SIZE 4096
#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL

#define DEFINE_SPINLOCK(x) spinlock_t x = __SPIN_LOCK_UNLOCKED(x)

enum states {
OFFLINE = 0,
IDLE,
LOADING,
UP,
DOWN
};

char *statusMessages[] = {"OFFLINE","IDLE","LOADING","UP","DOWN"};

enum eventType {
PASSENGER = 1,
ELEVATOR_MOVE,
ELEVATOR_OPEN,
PROC,
STOP
};
char *mBuffer;
static int floorMoves = 0;

// list events to be processed by kernel thread
LIST_HEAD(events_list);
DEFINE_SPINLOCK(events_lock);

// module does not require recompile of kernel.

struct list_head floors[10];
struct list_head exits[10];
struct task_struct *elevOp_id;


typedef struct Elevator {
	struct list_head lh;
	int totalWeight;
	int totalPassengers;
	int currentIndex;
	int destinationFloor;
	int status;
} Elevator;

bool elevatorStarted = false;
static int read_p;
static char *message;
static struct file_operations fops;
Elevator elevator;

bool needsToStop;
bool direction = 1;
static int movement = 1;
int serviced[10];
int floorCount[10];

int print_elevator(void) {

	int i;
	Elevator *elevator;
	struct list_head *temp;

	char *buf = kmalloc(sizeof(char) * 100, GFP_KERNEL);
	if (buf == NULL) {
		printk(KERN_WARNING "Elevator NULL BUFFER");
		return -ENOMEM;
	}

	/* init message buffer */
	strcpy(message, "");

	/* headers, print to temporary then append to message buffer */
	sprintf(buf, "Total count is: %d\n", elevator->totalPassengers);       strcat(message, buf);
	//sprintf(buf, "Total length is: %d\n", elevatortotal_length);   strcat(message, buf);
	sprintf(buf, "Total weight is: %d\n", elevator->totalWeight);   strcat(message, buf);
	//sprintf(buf, "Animals seen:\n");                               strcat(message, buf);

	/* print entries */
	i = 0;
	//list_for_each_prev(temp, &animals.list) { /* backwards */
	list_for_each(temp, &elevator->lh) { /* forwards*/
		elevator = list_entry(temp, Elevator, lh);

		/* newline after every 5 entries */
		if (i % 5 == 0 && i > 0)
			strcat(message, "\n");

		//sprintf(buf, "%s ", elevator->name);
		strcat(message, buf);

		i++;
	}

	/* trailing newline to separate file from commands */
	strcat(message, "\n");

	kfree(buf);
	return 0;
}
int printP(void){

//strcpy(message, "");
strcpy(message, "");


//sprintf(buf, "Total count is:0");       
strcat(message, mBuffer);
return 0;
}

/********************************************************************/

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file) {

	printk("proc called open");
	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "elevator_proc_open");
		return -ENOMEM;
	}
	//char *proc_test = "this is a proc_open test";
	
	//strcpy(message, proc_test);
	//return print_elevator();
	return printP();
}

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {

	printk("proc called read");
	int len = strlen(message);
	
	read_p = !read_p;
	if (read_p)
		return 0;
		
	copy_to_user(buf, message, len);
	return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file) {
	
	printk("proc called release");
	kfree(message);
	return 0;
}

/********************************************************************/


void initElevator(Elevator e){
//e->list = NULL; HOW DO I INIT THIS LIST?
//events_list = kmalloc(sizeof(struct list_head), __GFP_RECLAIM);
e.status = "ONLINE";
e.totalWeight = 0;
e.totalPassengers = 0;
e.currentIndex = 0;




}


DECLARE_WAIT_QUEUE_HEAD(wq);

// structure describing the event to be processed
typedef struct event {
    struct list_head lh; 
    int eventType; 
    void *data; 		//Assign void pointer to correct struct for event type
} Event; 

typedef struct passenger {
struct list_head lh;
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
kthread_stop(elevOp_id);

printk("elevOp stopped by kthread_stop");
}

void printPassengers(struct list_head *lh){

Passenger *p = (Passenger *)kmalloc(sizeof(Passenger), GFP_KERNEL);

list_for_each_entry(p, lh, lh){

printk("Initial floor: %d\n Destination Floor: %d\n type: %d, name:%s\n\n ",p->initialFloor, p->destinationFloor, p->type, p->name);



	}

	printk("exiting printPassengers");

}

int countList(struct list_head* hdr){
struct list_head *p;
int i = 0;
list_for_each(p, hdr)
{
i++;
} /* End for list */
return i;
}


void elevatorReport(void){
char *buffer = (char *)kmalloc(sizeof(char) * 512, __GFP_RECLAIM);
buffer[0] = 0;

sprintf(buffer, "Elevator report: status: %s current floor: %d, elevator load: weight: %d, passengers: %d\n", statusMessages[elevator.status], elevator.currentIndex + 1, elevator.totalWeight/2, elevator.totalPassengers);

strcat(mBuffer, buffer);
kfree(buffer);
}

void floorReport(int floorIndex){
char *buffer = (char *)kmalloc(sizeof(char) * 512, __GFP_RECLAIM);
buffer[0] = 0;

sprintf(buffer, "Floor %d: passengers waiting: %d, passengers serviced %d\n", floorIndex+1, countList(&floors[floorIndex]), serviced[floorIndex]);

strcat(mBuffer, buffer);
kfree(buffer);
}

void open(void){
/* Declare some temporary pointers */
struct list_head *temp;
struct list_head *dummy;
int tempWeight;
int tempLoad;


Passenger *p = (Passenger *)kmalloc(sizeof(Passenger), GFP_KERNEL);

//if(list_empty(&floors[elevator.currentIndex])){

//return 0;

//}

/* Use this since you need to change the pointers */
//    list_for_each_entry(p, &elevator.lh, lh){
//
//        if (elevator.currentFloor ==  p->destinationFloor) {
//            list_del_init(temp); /* init ver also reinits list */
//            kfree(p); /* remember to free alloced data */
//       }
//    }

//taking passenegrs off the elevator

while(!list_empty(&exits[elevator.currentIndex])){

	p = list_last_entry(&exits[elevator.currentIndex], Passenger, lh);
	printk("p has been assigned; address: %x\n", p);
	if(p){
	printk("passenger exiting elevator\n");
	elevator.status = LOADING;
	list_del_init(&p->lh);
	elevator.totalWeight = elevator.totalWeight - p->weight;
	elevator.totalPassengers -= p->passengerCount;

	//kfree(p);
	}

}


//load passengers onto elevator
tempWeight = elevator.totalWeight;
tempLoad = elevator.totalPassengers;
while(!list_empty(&floors[elevator.currentIndex]) && elevator.totalWeight < 30 && elevator.totalPassengers < 10){


printk("planning to load passengers on floors %d\n", elevator.currentIndex+1);
p = list_last_entry(&floors[elevator.currentIndex], Passenger, lh);

tempWeight += p->weight;
tempLoad += p->passengerCount;

if(tempWeight > 30 || tempLoad > 10){
break;
}


elevator.totalWeight = tempWeight;
elevator.totalPassengers = tempLoad;
elevator.status = LOADING;

serviced[elevator.currentIndex]++;

printk("inserted address:%x into exits #%d \n", p, (p->destinationFloor)-1);
list_move_tail(&p->lh, &exits[p->destinationFloor-1]);
//list_del_init(&p->lh);
//p= list_last_entry(&exits[elevator.currentIndex], Passenger, lh);

//printk("inserted address:%x into exits #%d \n", p, elevator.currentIndex +1);
	}


//kfree(p);
}	




int eventHandler(Event *ev){



    //Passenger *p = NULL;
    
    ////printk("ENTERING EVENT HANDLER");
    if(ev->eventType != NULL){
    ////printk("ev->eventType is not NULL");
    }

    printk("Event recieved: %d", ev->eventType);
    switch(ev-> eventType){
    ////printk("in eventType switch");


        case PASSENGER: {

    ////printk("in passenger switch");

        //Assign void pointer to passenger struct
        
        Passenger *p = (Passenger *)ev->data;

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
                p->name = "room service";
                break;

            case BELLHOP:
                p->passengerCount = 2;
                p->weight = 8;
                p->name = "bellhop";
                break;

            default:
                return 0;
        }
        //WHAT FLOOR ARE THEY GOING TO
        
        ////printk("About to put passenger onto floor queue");

        list_add(&p->lh, &floors[p->initialFloor -1]);
        //printPassengers(&floors[p->initialFloor -1]);

        ////printk("successfully put passenger onto floor queue");
            }

            
            break;

             

case ELEVATOR_MOVE: {
    //printk("Recieved elevator move event");
    int i = elevator.currentIndex;
    int starting = i;
    bool exit = false;
    static int movement = 1;
    
    
    bool waiting = false;
    
    while (!waiting) {
        i += movement;
	bool floorWating = !list_empty(&floors[i]);
	bool exitWating =  !list_empty(&exits[i]);

        waiting = floorWating || exitWating;
        printk("waiting %d : %d", i+1, waiting);
	printk("Bools: floorWating: %d exitwating: %d", floorWating, exitWating);
        if(waiting){
            elevator.currentIndex = i;
            printk("found waiting passenger on floor %d ", i+1);
        }
        
        
        if ( i == 9 || i == 0 ) {
            movement = (movement == 1) ? -1 : 1;
	elevator.status = (movement == 1) ? UP : DOWN;
        }
        
        if ( i == starting) {
		elevator.status = IDLE ;// looked at all cases exit...
            break;
        }
        
        
    }
    
    
//    if(direction){
//        for(i; i <=9; i++){
//            bool empty = list_empty(&floors[i]
//            printk("empty %d : %d", i, empty);
//            if(!empty){
//                elevator.currentIndex = i;
//                printk("found waiting passenger on floor %d ", i+1);
//
//                if(i == 9){
//                    //go back down
//                    direction = 0;
//                }
//                exit = true;
//            }
//            if (exit)
//                break;
//        }
//    }else{
//        for(i; i >= 0; i--){
//            if(!list_empty(&floors[i])){
//                elevator.currentIndex = i;
//
//                if(i == 0){
//                    //go back up
//                    direction = 1;
//                }
//               exit = true;
//            }
//            if (exit)
//                break;
//        }
//    }

printk("%d: moving elevator to floor %d ", floorMoves++, i+1);
                           
        }
            break;

        case ELEVATOR_OPEN:
        printk("opening elevator on floor %d", elevator.currentIndex + 1);
            open();

        ////printk("Done wth open call");
            break;

        case STOP:
         needsToStop = true;
        printk("IN STOP CASE");

 
        //if(list_empty(&elevator.lh){

        
        //code block to handle emptying elevator before going offlline.

        //}
        return 1;
        

            break;
	case PROC:{

printk("proc event handled");
mBuffer[0] = 0;
//strcpy(message, "");

int i = 0;
for(i; i < 9; i++){
floorReport(i);
	}

//elevator report
elevatorReport();

}
   
	break;

        default:
            return 0;


    }

////printk("exiting eventHandler");
return 0;
}








Event* get_next_event(void)
{
//printk("entering get_next_event");

//int size = list_empty(&events_list);

if(list_empty(&events_list)){
return NULL;
}
//printk("%d\n", size);

//if(size == 0){
//return NULL;
//}

	Event *e;
	spin_lock(&events_lock);
	//printk("about to assign e");
	e = list_last_entry(&events_list, Event, lh);
	//printk("e has been assigned; address: %x\n", e);
	if(e){ //should there be braces here?
	

	//printk("about to delete entry from events_list\n");
	list_del_init(&e->lh);
	

}

	//printk("about to spin_unlock");
	spin_unlock(&events_lock);

//printk("exiting get_next_event");

	return e;
}

void send_event(Event *ev)
{
	//printk("entering send_event");
	spin_lock(&events_lock);
	//printk("passed spin_lock()");
	//printk("ev: %x, events_list: %x", ev, &events_list);
    list_add(&ev->lh, &events_list);
    	//printk("passed list_add()");
    spin_unlock(&events_lock);
    //printk("passed spin_unlock()");
    wake_up(&wq);

//rintk("after wakeup");
}

Event *event(int type) {
    Event *event = (Event *)kmalloc(sizeof(Event), GFP_KERNEL);
    event->eventType = type;
    return event;
}


int elevator_operator(void *data){

Event *elevatorMove;
Event *elevatorOpen;
Event *procEvent;
    
    //printk("Starting elevator operator thread.");

    while(!kthread_should_stop()){

	send_event(event(PROC));
	ssleep(1);

	send_event(event(PROC));
	ssleep(1);

        
       //ssleep(2);

        //printk("Sending elevatorMove");
       elevatorMove = (Event *)kmalloc(sizeof(Event), GFP_KERNEL);
       elevatorMove->eventType = ELEVATOR_MOVE;
       send_event(elevatorMove);

        
        //printk("Sending elevatorOpen");
       elevatorOpen = (Event *)kmalloc(sizeof(Event), GFP_KERNEL);
       elevatorOpen->eventType = ELEVATOR_OPEN;
       send_event(elevatorOpen);
	
	send_event(event(PROC));
	ssleep(1);

    }
    
    //printk("Ending elevator operator thread.");
	do_exit(0);
	return 0;

}



int my_thread_f(void *data)  //Function that is passed into kthread_run 
{
	int p = 0;
        Event *e;
	initElevator(elevator);
    
    
	while (true) {
	wait_event(wq, (e = get_next_event()));
	
	/* Event processing */
	//printk("about to call event handhler");
	

	if (eventHandler(e) == 1)
		break;
		
	}
	
	shutdownElevator();
	do_exit(0);
}

long my_request_call(int p1, int p2, int p3) {
    printk(KERN_NOTICE "%s: Your int is %d,  %d, %d\n", __FUNCTION__, p1, p2, p3);
        Event *e = (Event *)kmalloc(sizeof(Event), GFP_KERNEL);
	
	Passenger *p = (Passenger *)kmalloc(sizeof(Passenger), GFP_KERNEL);

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

	Event *e = (Event *)kmalloc(sizeof(Event),GFP_KERNEL);
//	if(!e){
//	printk("Event from my_stop_call is null");
//}	
	e -> eventType = STOP;

	send_event(e);	
//stop call

    return 337;
}

long my_start_call(void) {
	printk(KERN_NOTICE "%s: \n", __FUNCTION__);



mBuffer = kmalloc(sizeof(char) * 4096, __GFP_RECLAIM);
strcpy(mBuffer, "");

	
	if(!elevatorStarted){
	//add boolean to keep track of if elevator is online or offline
	/* start and start kthread */
	//printk("Elevator has been started from module");	
	kthread_run(my_thread_f, NULL, "%s-kthread%d", "elevator", 0); //infinite loop, breaks when stop is TRUE

	//printk("starting elevator operator thread");
	elevOp_id = kthread_run(elevator_operator, NULL, "%s-kthread%d", "elevOp", 0);

	elevatorStarted = true;
//----------------------------------------------------
//add elevator event
////printk("about to malloc elevator event");


////Event *elevatorEvent = (Event *)kmalloc(sizeof(Event),GFP_KERNEL);
////elevatorEvent -> eventType = ELEVATOR_MOVE;

////printk("about to send elevator event");
 
////send_event(elevatorEvent);


//------------------------------------------------------
//add passengers


////printk("about to malloc passenger");
//Passenger *p1 = (Passenger *)kmalloc(sizeof(Passenger), GFP_KERNEL);






///my_request_call(1, 3, 4);
///my_request_call(1, 5, 7);
///my_request_call(1, 8, 10);



//p1-> initialFloor = 3;
//p1-> destinationFloor = 5;
//p1->type = ADULT; 

//Event *insertEvent1 = (Event *)kmalloc(sizeof(Event),GFP_KERNEL);
//insertEvent -> eventType = PASSENGER;

//insertEvent-> data = p;

/////printk("about to send passenger event");

//send_event(insertEvent);


//------------------------------------------------------


//stop call

////	printk(KERN_NOTICE "%s: \n", __FUNCTION__);

////	printk("Stopping elevator");

////	Event *e = (Event *)kmalloc(sizeof(Event),GFP_KERNEL);
//	if(!e){
//	printk("Event from my_stop_call is null");
//}	
////	e -> eventType = STOP;

////	send_event(e);	
//stop call



	return 0;
	} else {
	return 1;
	}
	////printk("Elevator ");


	
}



// puts my_test_call into STUB
static int hello_init(void) {
	STUB_start_elevator = my_start_call;
    	STUB_issue_request = my_request_call;
    	STUB_stop_elevator = my_stop_call;
 	int initCounter = 0;

	elevator.status = OFFLINE;

	for(initCounter; initCounter <= 9; initCounter++){
	INIT_LIST_HEAD(&floors[initCounter]);
	INIT_LIST_HEAD(&exits[initCounter]);



}

	//Passenger *pEx = (Passenger *)kmalloc(sizeof(Passenger),GFP_KERNEL);

	//pEx -> type = 1;
	//pEx->initialFloor = -5;
	//pEx->destinationFloor = 2;
	
	 //list_add(&pEx->lh, &exits[1]);

	fops.open = elevator_proc_open;
	fops.read = elevator_proc_read;
	fops.release = elevator_proc_release;
	
	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk(KERN_WARNING "elevator\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	//set bool stop to true, send event to stop
	return 0;
}


// puts null into the STUBS
static void hello_exit(void) {

//kthread_stop(elevOp_id);
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

remove_proc_entry(ENTRY_NAME, NULL);
//printk(KERN_NOTICE "Removing /proc/ %s.\n", ENTRY_NAME);

}

module_init(hello_init);
module_exit(hello_exit);

