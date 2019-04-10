#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
// 0 is reading pipe
// 1 is writing pipe
int pipes[2] ;

void
parent_proc()
{
	char * buf = 0x0 ;
	ssize_t s ;
	size_t len = 0 ;
	// close reading pipe in parent proc, meaning that only reader is child
	close(pipes[0]) ;

	// getline receive line from stdin
	// get reference of char pointer
	while ((s = getline(&buf, &len, stdin)) != -1) {
		buf[s - 1] = 0x0 ;	// get rid of null (new line)

		ssize_t sent = 0 ;
		char * data = buf ;		

		while (sent < s) {
			sent += write(pipes[1], buf + sent, s - sent) ;
		}

		free(buf) ;
		// prepare for next getline
		buf = 0x0 ;
		len = 0 ;
	}
	// and then, close writing pipe
	close(pipes[1]) ;
}

void
child_proc()
{
	char buf[32] ;
	ssize_t s ;
	// close writing buffer in child proc, meaning that parent is the only one that writes
	close(pipes[1]) ;
	// and child read on. 
	// parameter : file descriptor (pipes[0]), 
	// buf is not input! but for placeholder on which kernel syscall will put the data to
	// up to 31
	// if s reaches 0, mean eof
	// repeat until reading pipe closes  
	while ((s = read(pipes[0], buf, 31)) > 0) {
		buf[s + 1] = 0x0 ;
		printf(">%s\n", buf) ;
	}
	exit(0) ;
}

int
main()
{
	pid_t child_pid ;
	int exit_code ;

	if (pipe(pipes) != 0) {
		perror("Error") ;
		exit(1) ;
	}
	printf("%d %d\n", pipes[0], pipes[1]) ;

	child_pid = fork() ;
	// if child case, child proc
	if (child_pid == 0) {
		child_proc() ;
	}
	// if parent case, parent proc
	else {
		parent_proc() ;
	}
	wait(&exit_code) ;

	exit(0) ;
}
