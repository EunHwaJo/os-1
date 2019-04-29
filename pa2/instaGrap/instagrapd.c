// Partly taken from https://www.geeksforgeeks.org/socket-programming-cc/
#include <arpa/inet.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>


char * port = NULL;
char * wport = NULL;
char * wip = NULL;
char ins[10][10] = {0x0, };
char outs[10][10] = {0x0, };
char outputs[10][10] = {0x0, };
char * ids[20] = {0x0, };
char * pws[20] = {0x0, };
char * codes[20] = {0x0, };
int cnt = 0;			// for multiple submitter sharing ids/pws/codes array
int right_cnt = 0 ;


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
	char tempbuf[1024];
	char * flag = "correct";
	char * wrong_flag = "reject";
	int i = 0;
	struct sockaddr_in waddr;
	int worker_fd;
	struct sockaddr_in saddr;
	int submitter_fd;
	char temp_codes[1024] = {0x0, };
	char * wdata ;
	int k ;
	int flag2= 0;
	char check_msg[1024] ;
	//	int right_cnt = 0;

	FILE * compare;
	compare = fopen("compare.txt", "a");
	if(compare == NULL) {
		printf("file open error\n");
		exit(1);
	}

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


	if(strncmp(data,"check",5) == 0 ) {
		int s = 0;
		int len = 0;
		char * msg_data = 0x0 ;
		sprintf(check_msg, "%d cases were corret! \n", right_cnt);
		msg_data =strdup(check_msg) ;
		len = strlen(msg_data) ;
		while(len > 0 && (s = send(conn, msg_data, strlen(check_msg), 0) > 0)) {
			msg_data += s ;
			len -= s ;
		}

		
}









		bzero(buf, 1024);

		strcat(data, "-");
		temp_id = strtok(data, "-");
		temp_pw = strtok(NULL, "-");
		temp_code = strtok(NULL, "-");

		printf("temp_id : %s\ntemp_pw : %s\ntemp_code :\n%s", temp_id, temp_pw, temp_code);


		for(i = 0; i < 20; i++) {
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
			flag2 = 1;
		}
		shutdown(conn, SHUT_WR);

		for(i = 0; i < 10; i++) {
			if (flag2 == 1) break;
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

			if( send(worker_fd, wdata, strlen(wdata), 0) < 0 ) {
				printf("send error\n");
				exit(1);
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

				printf("\nshould be output from worker >%s\n", wdata); 	
				if( strcmp(wdata, outs[i]) == 0) {
					printf("correted!\n");
					right_cnt++;
				}
				fprintf(compare, "%s\n", wdata);		
			}
		}

		fclose(compare);
		compare = fopen("compare.txt", "r");
		for(i = 0; i < 10; i++) {
			fgets(tempbuf, 10, compare);
			printf("tempbuf : %s\n", tempbuf);
			printf("outs[%d] : %s\n", i, outs[i]);
			if( strcmp(tempbuf, outs[i]) == 0) {
				printf("correct!\n");
				right_cnt++;
			}
		}
		printf("%d test cases have been passed\n", right_cnt);


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
			//char dir[100] = "/home/sihyungyou/os/pa2/instaGrap/";
			char dir[100] = "/home/s21700696/os-1/pa2/instaGrap/";
			int i = 0;
			char testcase_ans[255] = {0x0, };
			char testcase[255] = {0x0, };
			char casefile[10];
			char temp[10] ={0x0, };
			char casenum[5];
			char casenum_out[5];	
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

			for(i = 1; i <= 10; i++) {
				FILE * anscases;
				strcpy(testcase_ans, dir);
				snprintf(casenum_out, sizeof(casenum_out), "%d", i);
				strcat(testcase_ans, casenum_out);
				strcat(testcase_ans, ".out");
				printf("testcase_ans : %s\n", testcase_ans);

				anscases = fopen(testcase_ans, "r");
				if (anscases == NULL) {
					printf("testcase_ans file open fail\n");
					exit(1);
				}
				fgets(temp, 9, anscases);
				strcpy(outs[i-1], temp);
				printf("outs[%d] : %s\n", i-1, outs[i-1]);
				fclose(anscases);
			}

			for(i = 1; i <= 10; i++) {
				// initialize input, output files
				FILE * cases;
				strcpy(testcase, dir);
				snprintf(casenum, sizeof(casenum), "%d", i);
				strcat(testcase, casenum);
				strcat(testcase, ".in");
				printf("testcase : %s\n", testcase);

				cases = fopen(testcase, "r");
				if(cases == NULL) {
					printf("testcase file open fail\n");
					exit(1);
				}
				fgets(temp, 9, cases);	
				strcpy(ins[i-1], temp);
				printf("ins[%d] : %s\n", i-1, ins[i-1]);
				fclose(cases);	
			}

			for(i = 0; i < 20; i++) {
				ids[i] = "0";
				pws[i] = "0";
				codes[i] = "0";
			}

			listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
			if (listen_fd == 0)  { 
				perror("socket failed : "); 
				exit(EXIT_FAILURE); 
			}

			memset(&address, '0', sizeof(address)); 
			address.sin_family = AF_INET; 
			address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ;	// computer itself 
			address.sin_port = htons(atoi(port));

			if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
				perror("bind failed : "); 
				exit(EXIT_FAILURE); 
			} 
			while (1) {
				if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
					perror("listen failed : "); 
					exit(EXIT_FAILURE); 
				} else printf("mainlog\n"); 

				new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;

				if (new_socket < 0) {
					perror("accept"); 
					exit(EXIT_FAILURE); 
				} 
				pthread_t t1;
				if ( pthread_create(&t1, NULL, child_proc,(void*) &new_socket) < 0) {
					perror("thread create error!");
					exit(0);
				}

				pthread_join( t1, NULL);
			} 
		}
