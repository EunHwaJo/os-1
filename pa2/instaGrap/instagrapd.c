// Partly taken from https://www.geeksforgeeks.org/socket-programming-cc/

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
char * id = 0x0;
char * pw = 0x0;
char * codes = 0x0;
void
child_proc(int conn)
{	
	// conn is socket
	char buf[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ; // how many char comes : out of our control

	// it repeatedly recieves data thru conn (new socket)
	// here, recv is exactly same as read w/o last param
	// we need to count how many char coming in
	while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
		printf("loop log\n");
		buf[s] = 0x0 ;
		/*
		if (data == 0x0) {
			data = strdup(buf) ;	// string duplicate buf
			len = s ;		//update length as s
		}
		*/
		if (id == 0x0) {
			printf("id log: %d\n", s);
			id = strdup(buf);
		}
		else if (pw == 0x0) {
			printf("pw log: %d\n", s);
			pw = strdup(buf);
		}
		else if (codes == 0x0)  {
			printf("codes log: %d\n", s);
			codes = strdup(buf);
		}
		/*
		else {
			data = realloc(data, len + s + 1) ;	// if not first time, realloc for len+s+1 amount. building up messeges 
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}
		*/
	}
	// loop iterates until revc is 0, menaing connection closed, no more char is given
	// print recieved data
	printf(">%s\n", id) ;
	printf(">%s\n", pw) ;
	printf(">%s\n", codes) ;
	
	orig = data ;
	// len > 0 means recieves sth, 
	// repeat sending the data to conn (socket which is bidirectional channel) -> read / write rold both operated
	while (len > 0 && (s = send(conn, data, len, 0)) > 0) {
		// send is same as recv logic
		// but even for writing, we can't determnine amount of char
		data += s ;	// ignore sent part,
		len -= s ;	// keep sending until len reaches 0
	}
	// notify it's all sent, shutdown writing channel
	shutdown(conn, SHUT_WR) ;
	if (orig != 0x0) 
		free(orig) ;
}

int 
main(int argc, char const *argv[]) 
{ 
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	
	char buffer[1024] = {0}; 
	
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
	address.sin_port = htons(8123); 				// port in current pc, server!

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
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 

		// if messege has come, accept operation run. 
		// new socket is returned from accept operation
		// real data transfer occurs here
		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;

		// if socket is negative value, it meanse there's some problems => ignore or terminate
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

		// someone must be waiting for new comer, previous socket must be alive => create child. (fork) so parent still stays as listener
		// -> multiple communication in single server becomes possible
		if (fork() > 0) {
			// reference to the socket is also copied to child
			child_proc(new_socket) ;
		}
		else {
			close(new_socket) ;
		}
	}
} 

