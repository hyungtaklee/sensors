#include <stdio.h>
#include <wiringPi.h>
#include <time.h>

#define TRIG_PIN 4
#define ECHO_PIN 5

int main(void)
{
	long dist, duration;

	time_t start_time, end_time, time_diff;

	double time_diff_sec;	
	
	double distance;

	if (wiringPiSetup() == -1) {
		printf("Setup failed\n");
		return -1;
	}

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

		time_diff_sec = (double) (end_time - start_time) / CLOCKS_PER_SEC;

		printf("distance: %.3lf\n", time_diff_sec * 17000);

		delay(300);
	}

	return 0;
}	
