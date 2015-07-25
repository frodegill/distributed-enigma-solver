/*
 *  enigma-solver-server
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include <cstdio>
#include <cstring>
#include <fstream>
#include <ios>
#include <netdb.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <vector>


#define REFLECTOR_COUNT (2) //B and C
#define RING_COUNT (5)//I, II, III, IV and V
#define PACKET_COUNT (REFLECTOR_COUNT*RING_COUNT*(RING_COUNT-1)*(RING_COUNT-2))


#ifdef DEFAULT_PORT_AS_INT
# define DEFAULT_PORT     (((('E'-'A')*26*26*26*26*26 + \
                             ('N'-'A')*26*26*26*26 + \
                             ('I'-'A')*26*26*26 + \
                             ('G'-'A')*26*26 + \
                             ('M'-'A')*26 + \
                             ('A'-'A')) % (65536-1024)) + 1024)
#else
# define DEFAULT_PORT  "2720" //Same as above, as string
#endif

#define EQUAL (0)
#define BACKLOG (10)
#define RECV_BUFFER_LENGTH (8*1024)
#define DEFAULT_SEND_FLAGS (0)
#define DEFAULT_RECV_FLAGS (0)
#define MAX_CALC_TIME_SEC (60*60)


std::string g_words;
std::string g_encrypted_text;

char* g_recv_buffer;

struct PacketInfo {
	int m_packet;
	time_t m_start_time;
};
std::vector<PacketInfo*> g_packets;


bool ReadWordlist(const char* wordlist_filename)
{
	if (!wordlist_filename)
		return false;

	std::ifstream file(wordlist_filename, std::ios::in|std::ios::binary|std::ios::ate);
	if (file.is_open())
	{
		std::ifstream::pos_type wordlist_filesize = file.tellg();
		char* wordlist_filebuffer = new char[static_cast<int>(wordlist_filesize)+1];
		file.seekg(0, std::ios::beg);
		file.read(wordlist_filebuffer, wordlist_filesize);
		file.close();
		wordlist_filebuffer[wordlist_filesize] = 0;

		int pos = 0;
		//ToUpper
		while (wordlist_filebuffer[pos])
		{
			if ('a'<=wordlist_filebuffer[pos] && 'z'>=wordlist_filebuffer[pos])
			{
				wordlist_filebuffer[pos] -= 'a'-'A';
			}
			pos++;
		}

		int length;
		pos = 0;
		//Parse
		while (wordlist_filebuffer[pos])
		{
			length = 0;
			while ('A'<=wordlist_filebuffer[pos+length] && 'Z'>=wordlist_filebuffer[pos+length]) length++;
			if (2<=length)
			{
				if (!g_words.empty())
				{
					g_words.append(1, ',');
				}
				g_words.append(std::string(wordlist_filebuffer+pos, length));
			}

			pos += length+1;
		}

		delete[] wordlist_filebuffer;
		return true;
	}

	return false;
}

bool ReadEncryptedText(const char* encrypted_text_filename)
{
	if (!encrypted_text_filename)
		return false;

	std::ifstream file(encrypted_text_filename, std::ios::in|std::ios::binary|std::ios::ate);
	if (file.is_open())
	{
		std::ifstream::pos_type encrypted_text_filesize = file.tellg();
		char* encrypted_text_filebuffer = new char[static_cast<int>(encrypted_text_filesize)+1];
		file.seekg(0, std::ios::beg);
		file.read(encrypted_text_filebuffer, encrypted_text_filesize);
		file.close();
		encrypted_text_filebuffer[encrypted_text_filesize] = 0;

		int pos = 0;
		//Parse
		while (encrypted_text_filebuffer[pos])
		{
			if ('A'<=encrypted_text_filebuffer[pos] && 'Z'>=encrypted_text_filebuffer[pos])
			{
				g_encrypted_text.append(1, encrypted_text_filebuffer[pos]);
			}
			else if ('a'<=encrypted_text_filebuffer[pos] && 'z'>=encrypted_text_filebuffer[pos])
			{
				g_encrypted_text.append(1, encrypted_text_filebuffer[pos] - 'a'-'A');
			}
			pos++;
		}

		delete[] encrypted_text_filebuffer;
		return true;
	}
	
	return false;
}

int CreateSocket(const char* port_str) // Code based on Beej's Guide to Network Programming example
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
	struct addrinfo* res;
	int status;
	if (0 != (status=getaddrinfo(NULL, port_str, &hints, &res)))
		return -1;

	int socket_fd = -1;
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
	struct addrinfo* p;
	for (p = res; p != NULL; p = p->ai_next) {
		socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_fd < 0) { 
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(socket_fd, p->ai_addr, p->ai_addrlen) < 0) {
			close(socket_fd);
			continue;
		}
		break;
	}
	freeaddrinfo(res); // all done with this

	if (-1!=socket_fd && -1==listen(socket_fd, BACKLOG)) {
		socket_fd = -1;
	}
    
	fprintf(stdout, "\nListening on port %s\n", port_str);

	return socket_fd;
}

void HandleClient(int& /*packet*/, int client_fd)
{
	int received_bytes = recv(client_fd, g_recv_buffer, RECV_BUFFER_LENGTH, DEFAULT_RECV_FLAGS);
	g_recv_buffer[received_bytes] = 0;
	if (EQUAL == strncmp("NEW ", g_recv_buffer, 4))
	{
		int bytes_sent;
		const char* send_buffer = g_words.c_str();
		int send_buffer_length = strlen(send_buffer);
		if (-1 == (bytes_sent=send(client_fd, "WORDS ", 6, DEFAULT_SEND_FLAGS))) return;
		while (0<send_buffer_length)
		{
			if (-1 == (bytes_sent=send(client_fd, send_buffer, send_buffer_length, DEFAULT_SEND_FLAGS))) return;
			send_buffer += bytes_sent;
			send_buffer_length -= bytes_sent;
		}
		if (-1 == (bytes_sent=send(client_fd, "\r\n", 2, DEFAULT_SEND_FLAGS))) return;

		send_buffer = g_encrypted_text.c_str();
		send_buffer_length = strlen(send_buffer);
		if (-1 == (bytes_sent=send(client_fd, "TEXT ", 5, DEFAULT_SEND_FLAGS))) return;
		while (0<send_buffer_length)
		{
			if (-1 == (bytes_sent=send(client_fd, send_buffer, send_buffer_length, DEFAULT_SEND_FLAGS))) return;
			send_buffer += bytes_sent;
			send_buffer_length -= bytes_sent;
		}
		if (-1 == (bytes_sent=send(client_fd, "\r\n", 2, DEFAULT_SEND_FLAGS))) return;

	}
	else if (0 == strncmp("DONE ", g_recv_buffer, 5))
	{
	}

}

void MainLoop(int socket_fd)
{
	int packet = 0;
	while (PACKET_COUNT > packet)
	{
		//Wait for content/connections
		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof their_addr;
		fprintf(stdout, "Waiting...(%d/%d)\n", packet, PACKET_COUNT);

		int client_fd = accept(socket_fd, (struct sockaddr*)&their_addr, &addr_size);
		HandleClient(packet, client_fd);
		close(client_fd);

		packet++;
	}
}

void PrintUsage()
{
	fprintf(stdout, "\nParameters: <path to wordlist> <path to encrypted text> [TCP/IP listening port] (default port: 2720)\n\n"
									"Example: ./enigma-solver-server files/english_words.txt files/encrypted.txt\n\n");
}

int main(int argc, char* argv[])
{
	if (2 >= argc)
	{
		PrintUsage();
		return -1;
	}

	if (!ReadWordlist(argv[1]))
	{
		fprintf(stderr, "Reading wordlist failed\n");
		return -1;
	}

	if (!ReadEncryptedText(argv[2]))
	{
		fprintf(stderr, "Reading encrypted text failed\n");
		return -1;
	}

	int socket = CreateSocket(4 <= argc ? argv[3] : DEFAULT_PORT);
	if (-1 == socket)
	{
		fprintf(stderr, "Initializing server failed\n");
		return -1;
	}

	g_recv_buffer = new char[RECV_BUFFER_LENGTH+1];
	if (!g_recv_buffer)
	{
		fprintf(stderr, "Allocating RECV buffer failed\n");
		return -1;
	}
	
	MainLoop(socket);
	close(socket);
	
	delete[] g_recv_buffer;
	return 0;
}
