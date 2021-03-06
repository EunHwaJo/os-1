#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
// lock is for only one thread at a time
pthread_mutex_t 	m = PTHREAD_MUTEX_INITIALIZER ;
// but in what order?
// conditional variable in pthread -> as a wait & notifying mechanism (ice and break)
pthread_cond_t  	c1 = PTHREAD_COND_INITIALIZER ;
pthread_cond_t  	c2 = PTHREAD_COND_INITIALIZER ;

void *print_message_function1( void *ptr )
{
	char *message;
	message = (char *) ptr;

	int i = 0 ;
	for (i = 0 ; i < 3 ; i++)  {
		printf("%s\n", message) ;
		printf("%s\n", message) ;
		printf("%s\n", message) ;
		// print messege 3 times in a row, and then
		// lock -> signal to c2! (signal means break)
		pthread_mutex_lock(&m) ;
			pthread_cond_signal(&c2) ;
		pthread_mutex_unlock(&m) ;
		// in this lock, wait on c1
		pthread_mutex_lock(&m) ;
			pthread_cond_wait(&c1, &m) ;
		pthread_mutex_unlock(&m) ;
	}
}

void *print_message_function2( void *ptr )
{
	char *message;
	message = (char *) ptr;

	int i = 0 ;
	for (i = 0 ; i < 3 ; i++)  {
		// first, wait on c2 (c2 is ice now, wait for break)
		pthread_mutex_lock(&m) ;			// racy
			pthread_cond_wait(&c2, &m) ;
		pthread_mutex_unlock(&m) ;

			printf("%s\n", message) ;
			printf("%s\n", message) ;
			printf("%s\n", message) ;
		// after printing messeges, signal c1
		pthread_mutex_lock(&m) ;
			pthread_cond_signal(&c1) ;
		pthread_mutex_unlock(&m) ;
	}
}


int
main()
{
	pthread_t thread1, thread2;

	char *message1 = "Thread 1";
	char *message2 = "Thread 2";

	pthread_create( &thread1, NULL, print_message_function1, (void*) message1);
	pthread_create( &thread2, NULL, print_message_function2, (void*) message2);

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL); 

	exit(0);
}


