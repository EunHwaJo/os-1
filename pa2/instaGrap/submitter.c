#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h>

// for server.c, it recieves data and sned the data
// for client.c, it send out the data and recieves data

	int 
main(int argc, char const *argv[]) 
{ 
	struct sockaddr_in serv_addr; 
	int sock_fd ;
	int s, len ;
	char buffer[1024] = {0}; 
	char * data ;
	char c;
	char * id = NULL;
	char * ip_port = NULL;
	char * ip = NULL;
	char * port = NULL;
	char * pw = NULL;
	char filename[20];
	char path[100] = "/home/sihyungyou/os/pa2/instaGrap/";
	char buf[1024] = {0x0, };
	char sdbuf[500];
	FILE * fs;
	int i = 0;
	int fsize = 0;
	bzero(sdbuf, 500);

	// getopt
	while(( c = getopt(argc, argv, "n:u:k:")) != -1)
		switch(c) {
			case 'n' : // ip:port
				ip_port = optarg;
			case 'u' : // id
				id = optarg;
				break;
			case 'k' : // pw
				pw  = optarg;
				break;
			case '?' :
				printf("Unknown flag : %d", optopt);
				break;
		}

	strcpy(filename, argv[argc-1]);
	ip = strtok(ip_port, ":");
	ip_port = strtok(NULL, " ");
	port = ip_port;	
	printf("ip_port : %s, ip : %s, port : %d\n", ip_port, ip, atoi(port));
	printf("id : %s, pw : %s\n", id, pw);
	printf("filename : %s\n", filename);

	strcat(path, filename);
	fs = fopen(path, "r");
	if (fs == NULL) {
		printf("file open error, %s is not found\n", path);
		exit(1);
	}
	char new_buffer[1024];
	char new_buffer2[1024];

	while((fsize = fread(sdbuf, sizeof(char), 500, fs)) > 0) {
		//printf("paased code : %s\n", sdbuf);
		//printf("> file %s from clinet was sent!\n", path);
	}

	sprintf(new_buffer, "%s-%s-%s", id, pw, sdbuf);
	sprintf(new_buffer2, "%s-%s-%s", id, "wrong!!", sdbuf);

	while(1) {
		sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
		if (sock_fd <= 0) {
			perror("socket failed : ") ;
			exit(EXIT_FAILURE) ;
		} 

		memset(&serv_addr, '0', sizeof(serv_addr)); 
		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_port = htons(atoi(port));

		if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
			perror("inet_pton failed : ") ; 
			exit(EXIT_FAILURE) ;
		} 

		if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			perror("connect failed : ") ;
			exit(EXIT_FAILURE) ;
		}
	
		printf("new connection has been made\n");
		if (i == 0) {
			//printf("sending all-concat id-pw-codes test\n");
			if( send(sock_fd, new_buffer, strlen(new_buffer), 0) < 0) {
				printf("sending error\n");
				exit(0);
			}
			shutdown(sock_fd, SHUT_WR);
			printf("sent info with : %s\n", new_buffer);
		}
		else  {
			if (i == 5) break;
			if ( send(sock_fd, new_buffer, strlen(new_buffer2), 0) < 0) {
				printf("requesting error\n");
				exit(0);
			}
			shutdown(sock_fd, SHUT_WR);
			printf("sent request with : %s\n", new_buffer2);\

			if ( s = recv(sock_fd, buf, 10, 0) > 0) {
				printf("recved feedback : %s\n", buf);
				if ( strcmp(buf, "correct") == 0) {
					printf("logged in\n");
					break;
				}
				else {
					printf("rejected\n");
					break;
				}
			}
		}
		printf("i is %d\n", i);
		i++;
	}

} 

