// Partly taken from https://www.geeksforgeeks.org/socket-programming-cc/

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 

//char * id = 0x0;
//char * pw = 0x0;
//char * codes = 0x0;
char * port = NULL;
char * wport = NULL;
char * wip = NULL;
char ins[10][10] = {0x0, };
char * ids[20] = {0x0, };
char * pws[20] = {0x0, };
char * codes[20] = {0x0, };
int flags[20] = {0, };		// to decide weather incoming data iss id, pw, or code
int cnt = 0;			// for multiple submitter sharing ids/pws/codes array

	void
child_proc(int conn)
{	
	// conn is socket
	char buf[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ; // how many char comes : out of our control
	//char * id = 0x0;
	//char * pw = 0x0;
	//char * codes = 0x0;
	char * flag = "correct";
	int i = 0;
	// it repeatedly recieves data thru conn (new socket)
	// here, recv is exactly same as read w/o last param
	// we need to count how many char coming in

	// address for worker
	struct sockaddr_in waddr;
	int worker_fd;
	// for each user (indexed by cnt), 1: id, 2: pw, 3: codes
	printf("before loop : flags[%d] : %d\n", cnt, flags[cnt]);
	while( (s = recv(conn, buf, 1023, 0)) > 0) {
		flags[cnt]++;	
		buf[s] = 0x0;
		data = strdup(buf);

		printf(" data : %s\n", data);
		printf("cnt : %d, flags[%d] : %d\n", cnt, cnt, flags[cnt]);
		if(flags[cnt] == 1) {
			printf("now for id\n");
			ids[cnt] = data;
		}
		else if ( flags[cnt] == 2) {
			printf("now for pw\n");
			pws[cnt] = data;
			continue;
		}
		else if ( flags[cnt] == 3) {
			printf("now for codes\n");
			codes[cnt] = data;
			continue;
		}
		else if (flags[cnt] > 3) {
			printf("asked feedback\n");
			if(strcmp(pws[0], buf) == 0) {
				printf("pw matches\n");
				if(s = send(conn, flag, strlen(flag), 0) < 0) {
					printf("return error\n");
				}
				else printf("returned flag : %s\n", flag);
				break;
			}
			else printf("wrong pw\n");
		}
	}
	printf("ids[%d] : %s\n", cnt, ids[cnt]);
	printf("pws[%d] : %s\n", cnt, pws[cnt]);
	printf("codes[%d] : %s\n", cnt, codes[cnt]);
	printf("=======end of child proc=======\n");
	shutdown(conn, SHUT_WR);


	worker_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(worker_fd <= 0) {
		perror("worker socket failed : ");
		exit(EXIT_FAILURE);
	}

	// write data to worker
	memset(&waddr, '0', sizeof(waddr));
	waddr.sin_family=AF_INET;
	waddr.sin_port = htons(atoi(wport));
	if(inet_pton(AF_INET, wip, &waddr.sin_addr) <= 0 ){
		perror("inet_pton failed : ");
		exit(EXIT_FAILURE);
	}
	if(connect(worker_fd, (struct sockaddr *) &waddr, sizeof(waddr)) < 0) {
		perror("connect failed : ");
		exit(EXIT_FAILURE);
	}
	// try token..
	char temp_codes[1023] = {0x0, };

	if(connect(worker_fd, (struct sockaddr *) &waddr, sizeof(waddr)) < 0) {
		for(i = 0; i < 10; i++) {
			sleep(1);
			strcpy(temp_codes, codes[0]);
			strcat(temp_codes, "|");
			strcat(temp_codes, ins[i]);
			if ( send(worker_fd, temp_codes, strlen(temp_codes), 0 ) < 0) {
				printf("concat code sending error\n");
			}
			else printf("concat code sent : %s\n", temp_codes);
		}
	}
		//shutdown(worker_fd, SHUT_WR) ;
}

	int 
main(int argc, char const *argv[]) 
{ 
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char c;
	char * ip_port = NULL;
	char dir[100] = "/home/sihyungyou/os/pa2/instaGrap/";
	int i = 0;
	//getopt	
	while( ( c = getopt(argc, argv, "p:w:"))!= -1) {
		switch(c) {
			case 'p' : // port waiting for submitter
				port = optarg;
				break;
			case 'w' : // ip and port going out for worker
				ip_port = optarg;
				break;
			case '?' :
				printf("Unkown flag: %d", optopt);
				break;				
		}

	}
	printf("argv[argc-1] : %s\n", argv[argc-1]);
	strcat(dir, argv[argc-1]);
	wip = strtok(ip_port, ":");
	ip_port = strtok(NULL, " ");
	wport = ip_port;

	printf("port : %s wip : %s wport : %s, dir : %s\n", port, wip, wport, dir);

	// read files from directory
	char testcase[255] = {0x0, };
	//strcpy(testcase, dir);
	char casefile[10];
	char temp[10] ={0x0, };
	//char temp[10] = {0x0, };
	for(i = 1; i <= 10; i++) {
		// initialize input, output files
		FILE * cases;
		strcpy(testcase, dir);
		char casenum[5];
		snprintf(casenum, sizeof(casenum), "%d", i);
		strcat(testcase, casenum);
		strcat(testcase, ".in");	// testcases/i.in
		printf("testcase : %s\n", testcase);

		cases = fopen(testcase, "r");
		if(cases == NULL) {
			printf("testcase file open fail\n");
			return -1;
		}
		fgets(temp, 9, cases);		// read as string
		strcpy(ins[i-1], temp);
		printf("ins[%d] : %s\n", i-1, ins[i-1]);
		fclose(cases);	
	}

	// create socket file descriptor
	// by using same socket, we can read & write
	// first param : af_inet, internet protocol
	// second param : connection type, TCP -> guarantee my messege is well delivered
	// third param : protocol, 0 means IP
	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}

	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ;	// computer itself 
	//address.sin_port = htons(8123); 				// port in current pc, server!
	address.sin_port = htons(atoi(port));

	// bind listen fd socket to certain address "this computer", meaning peace server
	// this is kind of welcoming channel
	// bind socket listen fd with current PC + port
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	} 
	while (1) {
		// my port is opened
		// listening for messege
		// exchanging ip address focus because other many callers are waiting for responses. 
		// and open another port for communication by which they can communicate "date". this is kind of a TCP protocol 
		// increase cnt to ensure multiple submitter's id, pw, codes, arrays
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} else printf("mainlog\n"); 

		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;

		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

		if (fork() > 0) {
			// reference to the socket is also copied to child
			printf("new socket\n");	
			child_proc(new_socket) ;
		}
		else {
			printf("close new socket\n");
			close(new_socket) ;
		}
	}
}
