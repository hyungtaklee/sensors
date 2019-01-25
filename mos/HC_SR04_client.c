/* 
 * read sensor values and publish them using libmosquitto 
 *
 */
#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include <mosquitto.h>

#define TRIG_PIN 4
#define ECHO_PIN 5

#define MQTT_HOST "127.0.0.1"
#define MQTT_PORT 1883

static volatile int run = 1;

void sig_handler(int s);
void connect_callback(struct mosquitto *mosq, void *obj, int result);
void message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message);

int main(void)
{
	/* sensor variables */
	long dist, duration;
	time_t start_time, end_time, time_diff;
	double time_diff_sec;	
	double distance;

	char post[256];

	/* mosquitto variables */
	uint8_t reconnect = true;
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	
	if (wiringPiSetup() == -1) {
		printf("Setup failed\n");
		return -1;
	}

	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	digitalWrite(TRIG_PIN, 0);
	delay(1000);

	printf("Start!\n");

	mosquitto_lib_init();

	memset(clientid, 0, 24);
	snprintf(clientid, 23, "sensor_cli_%d", getpid());
	
	mosq = mosquitto_new(clientid, true, 0);

	if (mosq) {
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

	while (run) {
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
		snprintf(post, strlen(post), "%.3lf\n", distance);

		rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
		if (rc) {
			mosquitto_publish(mosq, NULL, "sensor/distance/test",
					strlen(post), post, 0, 0);
		}
		delay(300);
	} /* end of while (run) */
	} /* end of if (mosq) */
	mosquitto_destroy(mosq);

	else { /* if (!mosq) */
		printf("error: mosq initialization error\n");
	}
	digitalWrite(TRIG_PIN, 0);

	return 0;
}	

void sig_handler(int s)
{
	run = 0;
}
