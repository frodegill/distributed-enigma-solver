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


const char* g_hostname;
const char* g_port;
char* g_network_buffer;

bool g_done = false;

int g_bigrams[CHAR_COUNT][CHAR_COUNT];
int g_trigrams[CHAR_COUNT][CHAR_COUNT][CHAR_COUNT];
int g_quadgrams[CHAR_COUNT][CHAR_COUNT][CHAR_COUNT][CHAR_COUNT];

uint8_t* g_encrypted_text = NULL;
PacketInfo* g_reflector_ring_settings = NULL;

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
			fprintf(stderr, "Closed socket %d\n", socket_fd);
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

bool ParseWordlist(NetworkInfo& network_info)
{
	fprintf(stdout, "Generate bi-/tri-/quadgrams from wordlist:\n");
	memset(g_bigrams, 0, CHAR_COUNT*CHAR_COUNT*sizeof(int));
	memset(g_trigrams, 0, CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*sizeof(int));
	memset(g_quadgrams, 0, CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*sizeof(int));

	std::string word;
	if (!ParseString(network_info, word) || EQUAL!=word.compare("WORDS")) return false;

	while (network_info.m_parsed_pos < (network_info.m_available_bytes+network_info.m_remaining_bytes))
	{
		if (!ParseString(network_info, word))
			return false;

		fprintf(stdout, " \"%s\"\n", word.c_str());
		if (2<=word.length()) ParseBigram(word);
		if (3<=word.length()) ParseTrigram(word);
		if (4<=word.length()) ParseQuadgram(word);

		SkipCharacter(network_info, ',');
	}

	return true;
}

bool ParseEncryptedText(NetworkInfo& network_info)
{
	std::string text;
	if (!ParseString(network_info, text) || EQUAL!=text.compare("TEXT")) return false;
	if (!ParseString(network_info, text)) return false;

	fprintf(stdout, "Encrypted text:\n");
	fprintf(stdout, " \"%s\"\n", text.c_str());
	g_encrypted_text = new uint8_t[text.length()];
	size_t i;
	for (i=0; i<text.length(); i++)
	{
		g_encrypted_text[i] = text[i]-'A';
	}

	return (0 == network_info.m_remaining_bytes);
}

bool ParseSetting(NetworkInfo& network_info)
{
	std::string text;
	int setting;
	if (!ParseString(network_info, text)) return false;

	if (EQUAL==text.compare("DONE"))
	{
		g_done = true;
		return true;
	}

	if (EQUAL!=text.compare("SETTING")) return false;
	
	if (!ParseInt(network_info, setting)) return false;

	delete g_reflector_ring_settings;
	g_reflector_ring_settings = new PacketInfo(setting);

	fprintf(stdout, "Current packet: %c %c%c%c\n", g_reflector_ring_settings->m_reflector+'B',
	                                               g_reflector_ring_settings->m_rings[LEFT]+'1',
	                                               g_reflector_ring_settings->m_rings[MIDDLE]+'1',
	                                               g_reflector_ring_settings->m_rings[RIGHT]+'1');

	return (0 == network_info.m_remaining_bytes);
}

void MainLoop(int& socket_fd)
{
	NetworkInfo network_info;
	network_info.m_socket_fd = socket_fd;
	network_info.SetBuffer("NEW",3);
	network_info.m_available_bytes = network_info.m_remaining_bytes = 3;
	if (!StartSendBuffer(network_info) || 0!=network_info.m_remaining_bytes)
		return;

	network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
	if (!StartRecvBuffer(network_info) ||
	    !ParseWordlist(network_info) || 0<network_info.m_remaining_bytes)
	{
		return;
	}

	if (!StartRecvBuffer(network_info) ||
	    !ParseEncryptedText(network_info) || 0<network_info.m_remaining_bytes)
	{
		return;
	}

	while (!g_done)
	{
		network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
		if (!StartRecvBuffer(network_info) ||
		    !ParseSetting(network_info) || 0<network_info.m_remaining_bytes)
		{
			break;
		}

		close(socket_fd);
		fprintf(stderr, "Closed socket %d\n", socket_fd);
		socket_fd = -1;

		//Calculate

		//Create socket, send result, loop and read next packet info
		socket_fd = CreateSocket(g_hostname, g_port);
		if (-1 == socket_fd)
		{
			fprintf(stderr, "Connecting to server failed\n");
			return;
		}
		fprintf(stderr, "Created socket %d\n", socket_fd);

		int score = 0;
		int ring_key_setting = 0;
		std::string plugboard="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		std::string plaintext="TODO";
		sprintf(network_info.buf(), "DONE %d %d %d %s ", g_reflector_ring_settings->ToInt(), score, ring_key_setting, plugboard.c_str());
		network_info.m_available_bytes = strlen(network_info.const_buf());
		network_info.m_remaining_bytes = network_info.m_available_bytes+plaintext.length();
		if (!StartSendBuffer(network_info))
		{
			return;
		}

		network_info.SetBuffer(plaintext.c_str(), plaintext.length());
		network_info.m_available_bytes = plaintext.length();
		if (!ContinueSendBuffer(network_info) || 0!=network_info.m_remaining_bytes)
		{
			return;
		}
	}
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

	g_hostname = argv[1];
	g_port = (3 <= argc ? argv[2] : DEFAULT_PORT);
	int socket = CreateSocket(g_hostname, g_port);
	if (-1 == socket)
	{
		fprintf(stderr, "Connecting to server failed\n");
		return -1;
	}
	fprintf(stderr, "Created socket %d\n", socket);

	g_network_buffer = new char[NETWORK_BUFFER_LENGTH+1];
	if (!g_network_buffer)
	{
		fprintf(stderr, "Allocating network buffer failed\n");
		close(socket);
		fprintf(stderr, "Closeded socket %d\n", socket);
		return -1;
	}
	g_network_buffer[NETWORK_BUFFER_LENGTH] = 0;

	MainLoop(socket);
	if (-1 != socket)
	{
		close(socket);
		fprintf(stderr, "Closed socket %d\n", socket);
	}

	delete[] g_network_buffer;

	delete g_reflector_ring_settings;
	delete[] g_encrypted_text;
	return 0;
}
