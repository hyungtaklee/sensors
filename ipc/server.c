#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 1024
#define BUFFLEN 8096

struct sensor_data { /* 16 bytes */
	char sensor_name[64]; /* 8 bytes */
	double value; /* 8 bytes */
};

int main(int argc, char **argv)
{
	int listenfd, connfd;
	struct sockaddr_in servaddr, clientaddr;

	int clientlen;
	char tempip[20];

	struct sensor_data recv_data;

	if (argc != 2) {
		printf("usage: %s [server_port]\n", argv[0]);
		return 0;
	}
	
	/* check port number */
	if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65535) {
		printf("fatal: invalid port number\n");
		return 0;
	}

	/* privilege warn */
	if (getuid() <= 0 || getuid() != geteuid()) {
		if (atoi(argv[1]) > 1023) {
			printf("warning: running server with root privilege\n");
		} else {
			printf("warning: running server program "
			"with port number smaller than 1024\n");
		}
	}

	/* open socket */
	if (listenfd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
		fprintf(stderr, "fatal: socket error\n");
		return -1;
	}

	/* intialize servaddr structure */
	memset(&servaddr, 0, sizeof(servaddr));
	
	/* set value of servaddr */
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	
	/* bind socket and file descriptor */
	if (bind(listenfd, (struct sockaddr *) &servaddr,
		  sizeof(servaddr)) < 0) {
		printf("fatal: bind error\n");
		return 0;
	}
	
	/* listen */
	if ((listen(listenfd, LISTENQ)) < 0) {
		printf("fatal: listen error\n");
		return 0;
	}

	clientlen = sizeof(clientaddr);

	for ( ; ; ) {
		connfd = accept(listenfd, 
			(struct sockaddr *) &clientaddr, &clientlen);
		if (connfd < 0) {
			printf("fatal: accept error\n");
			return 0;
		}
		/* show connection information */
		inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr,
				tempip, sizeof(tempip));
		printf("server: connected with %s\n", tempip);
	
		/* read sensor data from client */
		read(connfd, &recv_data, sizeof(recv_data));
		printf("received: device[%s], value[%lf]\n",
				recv_data.sensor_name, recv_data.value);

		close(connfd);
	}
	close(listenfd);
	return 0;
}
