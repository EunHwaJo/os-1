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
	int lens[3] = {0x0, };

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
	//filename = argv[argc-1];
	strcpy(filename, argv[argc-1]);
	ip = strtok(ip_port, ":");
	ip_port = strtok(NULL, " ");
	port = ip_port;	
	printf("ip_port : %s, ip : %s, port : %d\n", ip_port, ip, atoi(port));
	printf("id : %s, pw : %s\n", id, pw);
	printf("filename : %s\n", filename);



	// also create socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	//serv_addr.sin_port = htons(8123);
	serv_addr.sin_port = htons(atoi(port));

	// arbitrary ip address. 127.0.0.1 means server itself (not necessarily itself) 
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 
	// here, not binding but connection got executed
	// os approach destination thru network and establish connection, and cascate connection to given socket
	// so after that, we can read & write through socket
	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
	// recieves string from user
//	scanf("%s", buffer) ;
	
	// testing file transmit
	char sdbuf[500];
	strcat(path, filename);
	lens[0] = strlen(id);
	lens[1] = strlen(pw);
	printf("id length : %d, pw length : %d\n", lens[0], lens[1]);
	//filename = "/home/sihyungyou/os/pa2/instaGrap/test.c";
	//printf("client is sending %s to the server..\n", path);
	
	FILE * fs = fopen(path, "r");
	if (fs == NULL){
		printf("file open error, %s is not found\n", path);
		exit(1);
	}
	bzero(sdbuf, 500);
	
	int i = 0;
	int fsize;
	while(i < 3) {
		usleep(10);
		// id
		if(i == 0) {
			printf("passing id: %s\n", id);
			printf("length id: %d\n", lens[0]);
			if(send(sock_fd, id, lens[0], 0) < 0) {
				printf("id pass err!\n");
			}
			++i;
		}
		// pw
		else if (i == 1) {	
			printf("passing pw: %s\n", pw);
			printf("length pw : %d\n", lens[1]);
			if(send(sock_fd, pw, lens[1], 0) < 0) {
				printf("pw pass err!\n");
			}
			++i;
		}
		// codes
		else if (i == 2) {
			while((fsize = fread(sdbuf, sizeof(char), 500, fs)) > 0) {
				if (send(sock_fd, sdbuf, fsize, 0) < 0) {
					printf("stderr!\n");
					break;
				}
			//bzero(sdbuf, 500);
			printf("fsize : %d\n", fsize);
			}
			printf("file %s from client was sent!\n", path);
			++i;
		}
	}
	printf("terminated loop!: %d\n", i);
/*
	while((fsize = fread(sdbuf, sizeof(char), 500, fs)) > 0) {
		if(send(sock_fd, sdbuf, fsize, 0) < 0) {
			printf("stderr!\n");
			break;
		}
		bzero(sdbuf, 500);
	}
	printf("file %s form client was sent!\n", path);
*/	

	// up to here
	/*
	data = buffer ;
	len = strlen(buffer) ;
	s = 0 ;
	// transmit string to server and listen what server says
	while (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}*/
	

	shutdown(sock_fd, SHUT_WR) ;

	char buf[1024] ;
	data = 0x0 ;
	len = 0 ;
	while ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
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
	//printf(">%s\n", data); 

} 

