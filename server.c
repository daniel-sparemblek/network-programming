
#include <sys/socket.h>
#include <unistd.h> // sleep
#include <netdb.h> // addrinfo
#include <string.h> // memset
#include <err.h> //errx
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#define MAXLEN 512

int Socket(int family, int type, int protocol);
struct addrinfo * Getaddrinfo(const char*hostname, const char*service, int family, int socktype);
void Bind(int sockfd,const struct sockaddr *myaddr,int addrlen);
void Recvfrom(int sock, char* buf, int size, struct sockaddr *cli, socklen_t *clilen);
void Sendto(int sock, void* buf, int size, struct sockaddr* cli, socklen_t clilen);
void Listen(int sock, int backlog);
int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t *addrlen);
void Read(int sockfd, char *buf, int max);
void Write(int sockfd, char *buf, int num);
	
	
void fixpayload(char *payload) {
	//adds :\n to end of payload
	int i=0;
	while (payload[i] != '\n') {
		i++;
	}
	payload[i] = ':';
	payload[i+1] = '\n';
}
	
	
int main(int argc, char *argv[]) {
	int ch;
	int udpsock, tcpsock;;
	char *tcp_service = "1234";
	char *udp_service = "1234";
	char payload[MAXLEN] = "";
	memset(payload, 0, MAXLEN);
	char buf[10];
	struct addrinfo *res;
	struct sockaddr cli;
	socklen_t clilen = sizeof(cli);
	struct pollfd pollfds[10];
	int poll_ret;

	while((ch = getopt(argc, argv, "t:u:p:")) != -1) {
		switch(ch){
			case 't' : 
				tcp_service = optarg;
				break;
			case 'u': 
				udp_service = optarg;
				break;
			case 'p' :
				strcpy(payload, optarg);
				strncat(payload, "\n", 1);
				fixpayload(payload);
				break;
			default: 
				errx(1, "Usage: ./server [-t tcp_port] [-u udp_port] [-p popis]\n");
				break;
			}
		}
		
	
	res = Getaddrinfo(NULL, udp_service, AF_INET, SOCK_DGRAM);
	udpsock = Socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	Bind(udpsock, res->ai_addr, res->ai_addrlen);
	
	res = Getaddrinfo(NULL, tcp_service, AF_INET, SOCK_STREAM);
	tcpsock = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	Bind(tcpsock, res->ai_addr, res->ai_addrlen);
	Listen(tcpsock, 10);
	
	pollfds[0].fd = 0;
	pollfds[0].events = POLLIN;
	
	pollfds[1].fd = udpsock;
	pollfds[1].events = POLLIN;
	
	pollfds[2].fd = tcpsock;
	pollfds[2].events = POLLIN;
	

	while (1) {
		poll_ret = poll(pollfds, 3, 5000);
		
		if(poll_ret < 0) {
			errx(2, "Fatal error - poll\n");
		} else if (poll_ret == 0) {
			continue;
		} else {
			if (pollfds[0].revents & POLLIN) { //zahtjev je na stdin
				char line[MAXLEN];
				fgets(line, MAXLEN, stdin);
				
				if(strcmp(line, "PRINT\n") == 0) {
					printf("%s", payload);
				} else if (strncmp(line, "SET ", 4) == 0) {
					strncpy(payload, line + 4, sizeof(line) -4);
					fixpayload(payload);
				} else if(strcmp(line, "QUIT\n") == 0) {
					close(udpsock);
					close(tcpsock);
					return 0;
				} else {
					printf("Wrong input.\n");
				}
			} else if (pollfds[1].revents & POLLIN) { //zahtjev je udp
				Recvfrom(udpsock, buf, sizeof(buf), &cli, &clilen);
				if(strncmp(buf, "HELLO\n", 6) == 0) {
					Sendto(udpsock, payload, sizeof(payload), &cli, clilen);
				}
			} else if(pollfds[2].revents & POLLIN) { //zahtjev je tcp
				int newsock;
				newsock = Accept(tcpsock, &cli, &clilen);
				Read(newsock, buf, sizeof(buf));
				if(strncmp(buf, "HELLO\n", 6) == 0) {
					Write(newsock, payload, sizeof(payload));
				}
				close(newsock);	
			} else { //doslo je do pogreske
				errx(3, "Fatal error - poll");
			}
		}
	}
	
	close(udpsock);
	close(tcpsock);
	return 0;
}
