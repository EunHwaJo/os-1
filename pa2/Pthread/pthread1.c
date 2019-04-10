#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// in linux, kernel helps multi threaded programming
void *print_message_function( void *ptr )
{
	char *message;
	message = (char *) ptr;
	printf("%s \n", message);
}

int
main()
{
	pthread_t thread1, thread2;
	// input value
	char *message1 = "Thread 1";
	char *message2 = "Thread 2";
	// initiate, create thread
	// print messege function
	pthread_create(&thread1, NULL, print_message_function/*  */, (void*) message1);
	pthread_create(&thread2, NULL, print_message_function, (void*) message2);
	// only after thread1 terminate -> thread2 join operation will return
	// only after thread2 terminate -> thread join will return (?)
	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL); 

	exit(0);
}


