#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/*
   using pipes, child_proc of listening socket becomes main
   so, in child of parent socket, make pipes connection -> fork (child pipe) 
 */
int pipes[2];
/*
   int
   parent_pipe()
   {
   printf("parent_pipe()\n");
   char buf;
//stdout	
// close stdin	
close(pipes[0]);

int fd = open("./output", O_RDONLY | O_CREAT, 0644);
dup2(fd, 1);
read(fd, buf, sizeof(buf));
return atoi(buf);
}*/
/*
   void
   child_pipe(char * testcase)
   {
   printf("child_pipe()\n");

// stdin	
// close stdout
close(pipes[1]);

int fd = open("output.c", O_WRONLY | O_CREAT, 0644);
dup2(fd, 0);

system("gcc -o output output.c");
execl("./output", testcase);

wait(0x0);

}*/
	void
child_proc(int conn)
{
	char buf[1024];
	char * data = 0x0, * orig = 0x0;
	int len = 0;
	int s;
	char * codes = 0x0;
	char * testcase = 0x0;
	char * temp = 0x0;
	pid_t child_pid;
	int result = 0;
	char outputbuf[10];
	while( (s = recv(conn, buf, 1023, 0)) > 0) {
		printf("recv loop\n");
		buf[s] = 0x0;
		if (data == 0x0) {
			data = strdup(buf) ;
			len = s ;
		}
		else {
			data = realloc(data, len + s + 1) ;
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}
	}
	temp = strdup(data);
	codes = strtok(temp, "|");
	temp = strtok(NULL, " ");
	testcase = temp;
	//printf("codes : %s\n", codes);
	printf("testcase : %s\n", testcase);
	// now, run the code with stdin of testcase

	FILE * codefile;
	codefile = fopen("output.c", "w");
	if(codefile == NULL) {
		printf("codefile error\n");
		exit(1);
	}
	else fprintf(codefile, "%s", codes);
	fclose(codefile);

	FILE * tcfile;
	tcfile = fopen("testcase.txt", "w");
	if(tcfile == NULL) {
		printf("tcfile error\n");
		exit(1);
	}
	else fprintf(tcfile, "%s", testcase);
	fclose (tcfile);

	system("gcc -o output output.c");

	if(pipe(pipes) != 0) {
		perror("Error");
		exit(1);
	}

	pipes[0] = open("testcase.txt", O_RDONLY | O_CREAT, 0644);
	dup2(pipes[0], 0);
	close(pipes[0]);

	pipes[1] = open("output.out", O_WRONLY | O_CREAT, 0644);
	dup2(pipes[1], 1);
	close(pipes[1]);

	execl("./output", "output", (char *) 0x0);

	printf("execl log\n");

	FILE * opf;
	opf = fopen("output.out", "r");
	if(opf == NULL){
		printf("opf error : not opened!");
		exit(1);
	}

	fscanf(opf, "%s", outputbuf) ; // assume there's no error
	printf("outputbuf: %s", outputbuf);
	fclose(opf);

	len = strlen(outputbuf) ;
	while (len > 0 && (s = send(conn, outputbuf, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}
	shutdown(conn, SHUT_WR) ;
	if(data != 0x0)
		free(data) ;
}


//printf("data: %s\n", data);	

/* get output from executed log
 *data = 0x0;
 FILE * fp2;
 fp2 = fopen("output.txt", "r");
 while( fgets(buf, 1023, fp2) > 0 ) {
 strcat(data, buf);
 }
 printf("%s\n", data);
 */


	int
main(int argc, char *argv[])
{
	int listen_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char c;
	char * port = 0x0;
	printf("this is worker.c\n");

	//getopt
	while( (c = getopt(argc, argv, "p:")) != -1) {
		switch(c) {
			case 'p' : // port coming from
				port = optarg; 
		}
	}

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == 0) {
		perror("socket failed : ");
		exit(EXIT_FAILURE);
	}	
	memset(&address, '0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(atoi(port));

	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : ");
		exit(EXIT_FAILURE);
	}	
	while(1) {
		printf("while\n");
		if (listen(listen_fd, 16) < 0) {
			perror("listen failed : ");
			exit(EXIT_FAILURE);
		}

		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);
		printf("new socket log\n");
		if (new_socket < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		if (fork () > 0) {
			printf("fork success!\n");
			child_proc(new_socket);
		}
		else {
			printf("close new socket\n");
			close(new_socket);
			break;
		}
	}
}
