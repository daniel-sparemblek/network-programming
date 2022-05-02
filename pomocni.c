#include <sys/socket.h>
#include <unistd.h> 
#include <netdb.h> 
#include <string.h> 
#include <err.h> 
#include <stdio.h>
#define MAXLEN 512

int Socket(int family, int type, int protocol) {
	int n;
	if((n = socket(family, type, protocol)) < 0) {
		errx(2, "Fatal Error - socket");
	}
	return(n);
}

struct addrinfo * Getaddrinfo(const char*hostname, const char*service, int family, int socktype) {
	int error;
	struct addrinfo hints, *res;
	
	memset(&hints, 0, sizeof hints);
	
	hints.ai_family = family; 
	hints.ai_socktype = socktype;
	hints.ai_flags = AI_PASSIVE; 
	
	if ((error = getaddrinfo(hostname, service, &hints, &res)) != 0) {
		errx(3, "Fatal Error - getaddrinfo");
	}
	return res;
}

void Bind(int sockfd,const struct sockaddr *myaddr,int addrlen) {
	int error;
	if((error = bind(sockfd, myaddr, addrlen)) != 0) {
		errx(6, "Fatal error - bind");
	}
	return;
}

void Recvfrom(int sock, char* buf, int size, struct sockaddr *cli, socklen_t *clilen) {
	int msglen;
	if ((msglen = recvfrom(sock, buf, size, 0, cli, clilen)) < 0) {
		errx(4, "Fatal error - recvfrom");
	}
	return;
}

void Sendto(int sock, void* buf, int size, struct sockaddr* cli, socklen_t clilen) {
	int error;
	if((error = sendto(sock, buf, size, 0, cli, clilen)) == -1) {
		errx(5, "Fatal error - sendto");
	}
	return;
} 

int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t *addrlen) {
	int newfd;
	
	if((newfd = accept(sockfd, cliaddr, addrlen)) == -1) {
		errx(6, "Fatal error - accept");
	}
	return newfd;
}

void Connect(int sock, struct sockaddr* addr, socklen_t addrlen) {
	int error;
	
	if((error = connect(sock, addr, addrlen)) == -1) {
		errx(7, "Fatal error - connect");
	}
	return;
}

void Listen(int sock, int backlog) {
	int error;
	
	if((error = listen(sock, backlog)) == -1) {
		errx(8, "Fatal error - listen");
	}
	return;
}

void Read(int sockfd, char *buf, int max) {
	int len;
	
	if((len = read(sockfd, buf, max)) == -1) {
		errx(9, "Fatal error - read");
	}
	return;
}

void Write(int sockfd, char *buf, int num) {
	int len;
	
	if((len = write(sockfd, buf, num)) != num) {
		errx(10, "Fatal error - write");
	}
	return;
}

void Setsockopt(int sockfd, int level, int sockname, const void *opval, socklen_t optlen) {
	if (setsockopt(sockfd, level, sockname, opval, optlen) == -1) {
			errx(11," Fatal error - setsockopt");
		}
	return;
}
