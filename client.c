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

#define CHAR_COUNT (26)

#define OVERFLOW_PROTECTION (5) //From n-1-1 to n+1+1, to avoid range checking when adjusting for ring- and key-setting
#define OVERFLOW_BASE (2) //Pointer to start of middle block



char* g_network_buffer;

int g_bigrams[CHAR_COUNT][CHAR_COUNT];
int g_trigrams[CHAR_COUNT][CHAR_COUNT][CHAR_COUNT];
int g_quadgrams[CHAR_COUNT][CHAR_COUNT][CHAR_COUNT][CHAR_COUNT];

uint8_t g_reflector_definitions[REFLECTOR_COUNT][OVERFLOW_PROTECTION*CHAR_COUNT];
uint8_t g_ring_definitions[RING_COUNT][OVERFLOW_PROTECTION*CHAR_COUNT];
uint8_t g_inverse_ring_definitions[RING_COUNT][OVERFLOW_PROTECTION*CHAR_COUNT];
uint32_t g_ring_turnover_positions[RING_COUNT];


void InitializeEnigma()
{
	int overflow, reflector, ring, ch;

	//Initialize reflectors
	char reflector_defs[][CHAR_COUNT+1] = {{"EJMZALYXVBWFCRQUONTSPIKHGD"},  // A
	                                       {"YRUHQSLDPXNGOKMIEBFZCWVJAT"},  // B
	                                       {"FVPJIAOYEDRZXWGCTKUQSBNMHL"}}; // C
	for (reflector=0; reflector<REFLECTOR_COUNT; reflector++) {
		for (overflow=0; overflow<OVERFLOW_PROTECTION; overflow++) {
			for (ch=0; ch<CHAR_COUNT; ch++) {
				g_reflector_definitions[reflector][overflow*CHAR_COUNT + ch] = reflector_defs[reflector][ch] - 'A';
			}
		}
	}

	//Initialize ring definitions
	char ring_defs[][CHAR_COUNT+1] = {{"EKMFLGDQVZNTOWYHXUSPAIBRCJ"},  // I
	                                  {"AJDKSIRUXBLHWTMCQGZNPYFVOE"},  // II
	                                  {"BDFHJLCPRTXVZNYEIWGAKMUSQO"},  // III
	                                  {"ESOVPZJAYQUIRHXLNFTGKDCMWB"},  // IV
	                                  {"VZBRGITYUPSDNHLXAWMJQOFECK"},  // V
	                                  {"JPGVOUMFYQBENHZRDKASXLICTW"},  // VI
	                                  {"NZJHGRCXMYSWBOUFAIVLPEKQDT"},  // VII
	                                  {"FKQHTLXOCBJSPDZRAMEWNIUYGV"}}; // VIII
	for (ring=0; ring<RING_COUNT; ring++) {
		for (overflow=0; overflow<OVERFLOW_PROTECTION; overflow++) {
			for (ch=0; ch<CHAR_COUNT; ch++) {
				g_ring_definitions[ring][overflow*CHAR_COUNT + ch] = ring_defs[ring][ch] - 'A';
			}
		}
	}
	for (ring=0; ring<RING_COUNT; ring++) {
		for (overflow=0; overflow<OVERFLOW_PROTECTION; overflow++) {
			for (ch=0; ch<CHAR_COUNT; ch++) {
				g_inverse_ring_definitions[ring][overflow*CHAR_COUNT + g_ring_definitions[ring][overflow*CHAR_COUNT + ch]] = ch;
			}
		}
	}

	//Initialize turnover positions
	g_ring_turnover_positions[0] = 1<<16;
	g_ring_turnover_positions[1] = 1<<4;
	g_ring_turnover_positions[2] = 1<<21;
	g_ring_turnover_positions[3] = 1<<9;
	g_ring_turnover_positions[4] = 1<<25;
	g_ring_turnover_positions[5] = 1<<12|1<<25;
	g_ring_turnover_positions[6] = 1<<12|1<<25;
	g_ring_turnover_positions[7] = 1<<12|1<<25;
}

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
	fprintf(stdout, "Connecting to %s:%s\n", s, port);

	freeaddrinfo(servinfo); // all done with this structure

	return socket_fd;
}

void ParseBigram(const std::string& word) {
	size_t i;
	for (i=0; i<=word.length()-2; i++)
	{
		g_bigrams[word[i]-'A'][word[i+1]-'A']++;
	}
}

void ParseTrigram(const std::string& word) {
	size_t i;
	for (i=0; i<=word.length()-3; i++)
	{
		g_trigrams[word[i]-'A'][word[i+1]-'A'][word[i+2]-'A'] += 10; //A trigram is worth more than a bigram. Using 10 as a factor
	}
}

void ParseQuadgram(const std::string& word) {
	size_t i;
	for (i=0; i<=word.length()-4; i++)
	{
		g_quadgrams[word[i]-'A'][word[i+1]-'A'][word[i+2]-'A'][word[i+3]-'A'] += 100; //A quadgram is worth more than a bigram. Using 100 as a factor
	}
}

bool ParseWordlist(int socket_fd, char*& network_buffer, size_t& buffer_pos)
{
	fprintf(stdout, "Generate bi-/tri-/quad-grams from wordlist\n");
	memset(g_bigrams, 0, CHAR_COUNT*CHAR_COUNT*sizeof(int));
	memset(g_trigrams, 0, CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*sizeof(int));
	memset(g_quadgrams, 0, CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*sizeof(int));

	std::string word;
	if (!ParseString(socket_fd, network_buffer, buffer_pos, word) || EQUAL!=word.compare("WORDS")) return false;
	
	while (true)
	{
		if (!ParseString(socket_fd, network_buffer, buffer_pos, word)) return false;
		if (EQUAL==word.compare("\n")) break;

		fprintf(stdout, "%s\n", word.c_str());
		if (2<=word.length()) ParseBigram(word);
		if (3<=word.length()) ParseTrigram(word);
		if (4<=word.length()) ParseQuadgram(word);

		SkipCharacter(socket_fd, ',', network_buffer, buffer_pos);
	}

	return true;
}

void InitializeWork(int socket_fd)
{
	if (!SendBuffer(socket_fd, "NEW\n", 4))
		return;

	size_t buffer_pos = 0;
	g_network_buffer[buffer_pos] = 0;
	ParseWordlist(socket_fd, g_network_buffer, buffer_pos);
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

	InitializeEnigma();

	int socket = CreateSocket(argv[1], 3 <= argc ? argv[2] : DEFAULT_PORT);
	if (-1 == socket)
	{
		fprintf(stderr, "Connecting to server failed\n");
		return -1;
	}

	g_network_buffer = new char[NETWORK_BUFFER_LENGTH+1];

	if (!g_network_buffer)
	{
		fprintf(stderr, "Allocating memory failed\n");
		close(socket);
		return -1;
	}

	InitializeWork(socket);

	MainLoop();
	close(socket);

	delete[] g_network_buffer;
	return 0;
}
