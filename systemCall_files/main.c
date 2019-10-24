#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#define __NR_TEST_CALL 350

int test_call() {
	return syscall(__NR_TEST_CALL);
}

int main(int argc, char **argv) {
//	if (argc != 3) {
//		printf("wrong number of args\n");
//		return -1;
//	}
	
//	int test = atoi(argv[1]);
	long ret = test_call();

	//printf("sending this: %d\n", test);

	if (ret == NULL)
		perror("system call error");
	else
		printf("Function successful. passed in: , returned %ld\n", ret);
	
	printf("Returned value: %ld\n", ret);	

	return 0;
}

