#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


void
handler(int sig)
{
	if (sig == SIGINT) {
		printf("\nsigint : %d\n", SIGINT);
		printf("sig : %d\n", sig);
		printf("Do you want to quit?") ;
		if (getchar() == 'y')
			exit(0) ;
	}
}

int
main()
{
	signal(SIGINT, handler) ;
	while(1) ;
}
