#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define TRIG_PIN 4
#define ECHO_PIN 5

#define MAXLINE 128

struct sensor_data {
	char sensor_name[64];
	double value;
};

int main(int argc, char **argv)
{
	/* variables for network communtiation */
	int sockfd, n;
	char recvline[MAXLINE + 1];
	struct sockaddr_in servaddr;

	/* variables for sensor */
	long dist, duration;
	time_t start_time, end_time, time_diff;
	double time_diff_sec;	
	double distance;

	struct sensor_data data;

	/* check argument */
	if (argc != 2) {
		printf("usage: %s [server_port]\n", argv[0]);
		return 0;
	}
	
	/* check port number */
	if (atoi(argv[1]) < 0 && atoi(argv[0]) > 65535) {
		printf("error: invalid port number\n");
		return 0;
	}
	
	/* setup wiringPi */
	if (wiringPiSetup() == -1) {
		printf("error: wiringPi setup failed\n");
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error: socket error\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
		printf("error: inet_pton error for %s\n", "127.0.0.1");
		return -1;
	}

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))< 0){
		printf("error: connect error\n");
		return -1;
	}

	/* initialize sensor */
	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	digitalWrite(TRIG_PIN, 0);
	delay(1000);

	printf("Start!\n");

	for (;;) {
		digitalWrite(TRIG_PIN, 0);
		delay(500); // write LOW for 500ms

		digitalWrite(TRIG_PIN, 1);
		delayMicroseconds(10);
		
		digitalWrite(TRIG_PIN, 0);
		
		while (digitalRead(ECHO_PIN) == 0)
			start_time = clock();

		while(digitalRead(ECHO_PIN) == 1) {
			end_time = clock();
		}
		time_diff_sec = (double)(end_time - start_time) 
					/ CLOCKS_PER_SEC;
		
		data.value = time_diff_sec * 17000;
		printf("distance: %.3lf\n", data.value);

		write(sockfd, data, sizeof(data));

		delay(300);
	}

	return 0;
}	
