#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

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
	printf("this is child process of worker\n");

	while( (s = recv(conn, buf, 1023, 0)) > 0) {
		printf("recv loop\n");
		buf[s] = 0x0;
		temp = strdup(buf);
		codes = strtok(temp, "|");
		temp = strtok(NULL, " ");
		testcase = temp;
		printf("codes : %s\n", codes);
		printf("testcase : %s\n", testcase);
		/*if(codes == 0x0) {
			codes = strdup(buf);
		}
		else if(codes != 0x0 && testcase == 0x0) {
			testcase = strdup(buf);
			codes = 0x0;
			//testcase = 0x0;
		}
		printf("codes : %s\n", codes);
		printf("testcase : %s\n", testcase);
		*/		
		/*	
			if (data == 0x0) {
			data = strdup(buf);
			len = s;
			} 
			else {
			data = realloc(data, len+s+1);
			strncpy(data+len, buf, s);
			len += s;	
			}
		 */	
	}
	//printf("data: %s\n", data);	
	
	/*
	FILE * fp;
	fp = fopen("output.c", "w");
	fprintf(fp, "%s", codes);
	fclose(fp);
	*/
	
	/*
	// run submitted test code
	system("gcc -o output output.c");
	system("./output > output.txt");

	// get output from executed log
	 *data = 0x0;
	 FILE * fp2;
	 fp2 = fopen("output.txt", "r");
	 while( fgets(buf, 1023, fp2) > 0 ) {
	 strcat(data, buf);
	 }
	 printf("%s\n", data);
	 */
}



	int
main(int argc, char const *argv[])
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
