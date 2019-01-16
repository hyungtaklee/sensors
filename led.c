#include <stdio.h>
#include <signal.h>
#include <wiringPi.h>

static volatile int run = 1;

void int_handler(int mmmmm) {
	run = 0;
}

int main(void)
{
	int i;

	if (wiringPiSetup() == -1) {
		printf("setup failed\n");
		return -1;
	}

	pinMode(3, OUTPUT);

	signal(SIGINT, int_handler);

	while (run) {
		digitalWrite(3, 1);
		delay(250);
		digitalWrite(3, 0);
		delay(250);
	}

	digitalWrite(3, 0);
	
	return 0;
}
