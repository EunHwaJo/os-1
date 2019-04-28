// Partly taken from https://www.geeksforgeeks.org/socket-programming-cc/
#include <arpa/inet.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>

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
int flags[20] = {0, };		// to decide wether incoming data iss id, pw, or code
int cnt = 0;			// for multiple submitter sharing ids/pws/codes array

	void*
child_proc(void* ptr)
{	int conn = *((int *) ptr);
	char buf[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ; 
	char * temp_id = 0x0;
	char * temp_pw = 0x0;
	char * temp_code = 0x0;
	char * flag = "correct";
	char * wrong_flag = "reject";
	int i = 0;
	struct sockaddr_in waddr;
	int worker_fd;

	printf("CHILD PROC cnt:  %d\n", cnt);
	while( (s = recv(conn, buf, 1023, 0)) > 0) {
		buf[s] = 0x0;
		if (data == 0x0) {
			data = strdup(buf);
			len = s;
		}
		else {
			data = realloc(data, len+s+1);
			strncpy(data+len, buf, s);
			data[len+s] = 0x0;
			len += s;
		}
	}
	bzero(buf, 1024);

	strcat(data, "-");
	//printf("NEW DATA : %s\n", data);
	// slice the string by token "-"
	temp_id = strtok(data, "-");
	temp_pw = strtok(NULL, "-");
	temp_code = strtok(NULL, "-");

	printf("temp_id : %s\ntemp_pw : %s\ntemp_code :\n%s", temp_id, temp_pw, temp_code);


	for(i = 0; i < 20; i++) {
		//printf("for log\n");
		//printf("temp_id : %s\n", temp_id);
		//printf("ids[%d] : %s\n",i, ids[i]);
		if( strcmp(temp_id, ids[i]) == 0) {
			// if exist,
			printf("login info exist\n");

			if( strcmp(temp_pw, pws[i]) == 0 ) {
				// if id && pw both exist, meaning login available
				printf("login success\n");
				if( send(conn, flag, strlen(flag), 0) < 0) {
					printf("feedback error\n");
					exit(0);
				}
				break;
			}
			else {
				// id matches, but pw is wrong
				printf("wrong pw\n");
				if( send(conn, wrong_flag, strlen(wrong_flag), 0) < 0) {
					printf("wrong feedback error\n");
					exit(0);
				}
				break;
			}
		} 
	}
	// if doenst exist, store the input data
	if (i == 20) {
		printf("stored!");
		ids[cnt] = temp_id;
		pws[cnt] = temp_pw;
		codes[cnt] = temp_code;
		cnt++;

	}
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
	// try token..
	char temp_codes[1024] = {0x0, };
	char * wdata ;
	int k ;

	for(i = 0; i < 10; i++) {
		sleep(3);
		memset(temp_codes, 0, sizeof(temp_codes)) ;
		if(connect(worker_fd, (struct sockaddr *) &waddr, sizeof(waddr)) < 0) {
			perror("connect failed?! : ");
			exit(EXIT_FAILURE);
		}
		strcpy(temp_codes, codes[0]);
		strcat(temp_codes, "|");
		strcat(temp_codes, ins[i]);

		wdata = temp_codes ;
		len = strlen(temp_codes) ;
		k = 0; 

		while (len > 0 && (k = send(worker_fd, wdata, len, 0)) > 0) {
			data += k ;
			len -= k ;
		}


		shutdown(worker_fd, SHUT_WR) ;

		char wbuf[1024] ;
		wdata = 0x0 ;
		len = 0;
		while ( (k = recv(worker_fd, wbuf, 1023, 0)) > 0 ) {
			wbuf[k] = 0x0 ;
			if (wdata == 0x0) {
				wdata = strdup(wbuf) ;
				len = k ;
			}
			else {
				wdata = realloc(wdata, len + k + 1) ;
				strncpy(wdata + len, wbuf, k) ;
				wdata[len + k] = 0x0 ;
				len += k ;
			}

		}
		printf("\nshould be output from worker >%s\n", wdata); 	

	}
}

	int 
main(int argc, char *argv[]) 
{ 
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char c;
	char * ip_port = NULL;
	//	char dir[100] = "/home/sihyungyou/os/pa2/instaGrap/";
	char dir[100] = "/home/s21700696/os-1/pa2/instaGrap/";
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

	for(i = 0; i < 20; i++) {
		ids[i] = "0";
		pws[i] = "0";
		codes[i] = "0";
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
		/*
		   if (fork() > 0) {
		// reference to the socket is also copied to child
		printf("new socket\n");	
		child_proc(new_socket) ;
		}
		 */
		// try multi thread
		//int * new_fd = malloc(sizeof(*new_fd));
		//new_fd = new_socket;
		pthread_t t1;
		if ( pthread_create(&t1, NULL, child_proc,(void*) &new_socket) < 0) {
			perror("thread create error!");
			exit(0);
		}

		pthread_join( t1, NULL);
		/*
		   else {
		   printf("close new socket\n");
		   close(new_socket) ;
		   }
		 */
	}
}
