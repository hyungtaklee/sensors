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
	printf("message callback\n");
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
	snprintf(clientid, 23, "test publisher"); /* set client id */
	mosq = mosquitto_new(clientid, true, 0); /* init new mosq obj */

	if (mosq) {
		/* set callback functions for each situations */
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

		rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
		
		if (run) {
			char topic[30];
			char message[256];

			printf("Enter topic name: ");
			scanf("%s", topic);

			printf("Enter message: ");
			scanf("%s", message);

			mosquitto_publish(mosq, NULL, topic, strlen(message),
					message, 0, 0);
		}

		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();

	return rc;
}
