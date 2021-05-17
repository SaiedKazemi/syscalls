#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

typedef void (*sighandler_t)(int);

void quit(int signum)
{
	exit (0);
}

/*
 * Ignore signal 1.
 * Exit on signal 2.
 */
int main(int argc, char *argv[])
{
	signal(1, SIG_IGN);
	signal(2, quit);
	while (1)
		sleep(10);
	return 0;
}
