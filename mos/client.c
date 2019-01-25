#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h> /* C99 */

#include <mosquitto.h>

#define MQTT_HOST "127.0.0.1"
#define MQTT_PORT 1883
#define TRUE 1
#define FALSE 0

static int run = 1;

void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc = %d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message)
{
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", message->payloadlen,
		(char *) message->payload, message->topic);

	mosquitto_topic_matches_sub(0, message->topic, &match);

	if (match) {
		printf("get m0essage for ADC topic\n");
	}
}

int main(int argc, char *argv[])
{
	uint8_t reconnect = true;
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	mosquitto_lib_init();

	memset(clientid, 0, 24);
	snprintf(clientid, 23, "rameon sashipsyo"); /* set client id */
	mosq = mosquitto_new(clientid, true, 0); /* init new mosq obj */

	if (mosq) {
		/* set callback functions for each situations */
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

		rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);

		/* "test" is topic name */
		mosquitto_subscribe(mosq, NULL, "hi", 0);

		while (run) {
			rc = mosquitto_loop(mosq, -1, 1);

			if (run && rc) {
				printf("connection error\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}
		}
		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();

	return rc;
}
