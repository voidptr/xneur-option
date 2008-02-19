
#include "functions.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

char* copy_pchar(const char *source, int len, bool setnull)
{
	if(source == NULL)
		return NULL;

	int str_len;
	if(len == 0)
		str_len = strlen(source) + 1;
	else
		str_len = len;

	char *copy = new char[str_len + setnull];
	memcpy(copy, source, str_len);
	if(setnull)
		copy[str_len] = '\0';

	return copy;
}

void error(const char *msg, ...)
{
	va_list params;
	va_start(params, msg);

	vprintf(msg, params);
	printf("\n");

	va_end(params);

	exit(EXIT_FAILURE);
}

void warning(const char *msg, ...)
{
	va_list params;
	va_start(params, msg);

	vprintf(msg, params);
	printf("\n");

	va_end(params);
}

void trap_signal(int sig, sg_handler handler)
{
	struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handler;

	if(sigaction(sig, &sa, NULL) == -1)
		error("Error %d occured int sigaction(%d)", errno, sig);
}

void send_signal(int pid, int sig)
{
	kill(pid, sig);
}
