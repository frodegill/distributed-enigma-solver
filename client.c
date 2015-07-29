/*
 *  enigma-solver-client
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>



char* g_network_buffer;


void* get_in_addr(struct sockaddr* sa) // Code from on Beej's Guide to Network Programming
{
  return (AF_INET==sa->sa_family) ? (void*)&(((struct sockaddr_in*)sa)->sin_addr) : (void*)&(((struct sockaddr_in6*)sa)->sin6_addr);
}

int CreateSocket(const char* hostname, const char* port) // Code based on Beej's Guide to Network Programming example
{
	int socket_fd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket_fd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	return socket_fd;
}

void InitializeWork(int /*socket_fd*/)
{
}

void MainLoop()
{
}

void PrintUsage()
{
	fprintf(stdout, "\nParameters: <server hostname> [server port] (default port: 2720)\n\n"
									"Example: ./enigma-solver-client localhost 2720\n\n");
}

int main(int argc, char* argv[])
{
	if (1 >= argc)
	{
		PrintUsage();
		return -1;
	}

	int socket = CreateSocket(argv[1], 3 <= argc ? argv[2] : DEFAULT_PORT);
	if (-1 == socket)
	{
		fprintf(stderr, "Connecting to server failed\n");
		return -1;
	}

	g_network_buffer = new char[NETWORK_BUFFER_LENGTH+1];
	if (!g_network_buffer)
	{
		fprintf(stderr, "Allocating network buffer failed\n");
		close(socket);
		return -1;
	}

	InitializeWork(socket);

	MainLoop();
	close(socket);

	delete[] g_network_buffer;
	return 0;
}
