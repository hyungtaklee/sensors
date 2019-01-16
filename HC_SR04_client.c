#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <curl/curl.h>
#include <string.h>

#define TRIG_PIN 4
#define ECHO_PIN 5

static volatile int run = 1;

void int_handler(int dummy_val);

int main(void)
{
	long dist, duration;
	time_t start_time, end_time, time_diff;
	double time_diff_sec;	
	double distance;

	char post[256];

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);


	signal(SIGINT, int_handler);

	if (wiringPiSetup() == -1) {
		printf("Setup failed\n");
		return -1;
	}

	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	digitalWrite(TRIG_PIN, 0);
	delay(1000);

	printf("Start!\n");

	while (run) {
		curl = curl_easy_init();

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

		time_diff_sec = (double) (end_time - start_time) / CLOCKS_PER_SEC;

		distance = time_diff_sec * 17000;

		printf("distance: %lf\n", distance);
	
	
		if (curl) {
			char post[256];	
			
			sprintf(post, "distance=%.3lf", distance);

			printf("DEBUG: post: %s\n", post);
			
			curl_easy_setopt(curl, CURLOPT_URL, "192.168.86.162:3000");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
		
			res = curl_easy_perform(curl);
		
			if (res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			}
			curl_easy_cleanup(curl);
		}
		delay(1000);
	}
	
	curl_global_cleanup();
	digitalWrite(TRIG_PIN, 0);

	return 0;
}	

void int_handler(int dummy_val)
{
	run = 0;
}
