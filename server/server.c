/*
 *  enigma-solver-server
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <ios>
#include <list>
#include <stdlib.h>
#include <unistd.h>


#define BACKLOG (10)

char* g_network_buffer;

std::string g_words;
std::string g_encrypted_text;

uint32_t g_max_score = 0;
uint32_t g_max_reflector_and_ring_settings = 0;
uint32_t g_max_ring_key_settings = 0;
std::string g_max_plugboard;
std::string g_max_plaintext;


std::list<PacketInfo> g_pending_packets;


void PrintUsage()
{
	fprintf(stdout, "\nParameters: <rotors (A/B/C/b/c)> [M4 thin ring (B/G)] <left rings (1/2/3/4/5/6/7/8)> <middle rings> <right rings> <path to wordlist> <path to encrypted text> [TCP/IP listening port]\nDefault port is 2720.\n\n"
									"Example: ./enigma-solver-server bc BG 12345678 12345678 12345678 files/english_words.txt files/encrypted.txt\n\n");
}

void CompressPlugboard(const std::string& plugboard, std::string& compressed_plugboard)
{
	compressed_plugboard.clear();
	compressed_plugboard += " [ ";
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		if (plugboard[i] > (i+'A'))
		{
			compressed_plugboard += (char)(i+'A');
			compressed_plugboard += '-';
			compressed_plugboard += (char)plugboard[i];
			compressed_plugboard += ' ';
		}
	}
	compressed_plugboard += ']';
}

void ResultString(std::string& result)
{
	std::string reflector_and_rings;
	PacketInfo max_packet;
	max_packet.FromInt(g_max_reflector_and_ring_settings);
	max_packet.ToString(reflector_and_rings);

	std::string ring_settings, key_settings;
	uint32_t value = g_max_ring_key_settings;
	uint32_t modulo = CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT;
	uint32_t c;
	uint8_t i;
	for (i=0; i<(2*ROTOR_COUNT); i++)
	{
		c = value/modulo;
		value -= c*modulo;
		modulo /= CHAR_COUNT;
		if (i<ROTOR_COUNT)
		{
			ring_settings += (char)(c+'A');
		}
		else
		{
			key_settings += (char)(c+'A');
		}
	}

	std::string compressed_plugboard;
	CompressPlugboard(g_max_plugboard, compressed_plugboard);

	char tmp_buffer[200];
	snprintf(tmp_buffer, 200, "%d, %s-%s-%s %s ",
					 g_max_score,
					 reflector_and_rings.c_str(), ring_settings.c_str(), key_settings.c_str(),
					 compressed_plugboard.c_str());

	result.clear();
	result.append(tmp_buffer);
	result.append(g_max_plaintext);
	result += '\n';
}

void PrintResult()
{
	std::string result_string;
	ResultString(result_string);

	fprintf(stdout, "Best score: %s\n", result_string.c_str());
}

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

		size_t pos = 0;
		//ToUpper
		while (wordlist_filebuffer[pos])
		{
			if ('a'<=wordlist_filebuffer[pos] && 'z'>=wordlist_filebuffer[pos])
			{
				wordlist_filebuffer[pos] -= 'a'-'A';
			}
			pos++;
		}

		size_t length;
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

		size_t pos = 0;
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
#ifdef DEBUG
			fprintf(stderr, "Closed socket %d\n", socket_fd);
#endif
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

void RemovePendingPacketInfo(uint32_t reflector_and_rings_settings)
{
	for (std::list<PacketInfo>::iterator iter = g_pending_packets.begin(); iter != g_pending_packets.end(); ++iter)
	{
		PacketInfo pending_packet_info = *iter;
		if (reflector_and_rings_settings == pending_packet_info.m_packet_number)
		{
			g_pending_packets.erase(iter);
			break;
		}
	}
}

void SendPacket(NetworkInfo& network_info)
{
	if (g_pending_packets.empty())
	{
		network_info.SetBuffer("DONE",4);
		network_info.m_available_bytes = network_info.m_remaining_bytes = 4;
		StartSendBuffer(network_info);
	}
	else
	{
		PacketInfo sent_packet = *g_pending_packets.begin();
		g_pending_packets.pop_front();
		g_pending_packets.push_back(sent_packet);

		network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
		sprintf(network_info.buf(), "SETTING %d", sent_packet.m_packet_number);
		network_info.m_available_bytes = network_info.m_remaining_bytes = strlen(network_info.const_buf());
		if (!StartSendBuffer(network_info) || 0!=network_info.m_remaining_bytes)
			return;

		std::string sent_packet_str;
		sent_packet.ToString(sent_packet_str);
		fprintf(stdout, "Sent packet %d (%s)\n", sent_packet.m_packet_number, sent_packet_str.c_str());
	}
}

void HandleClient(NetworkInfo& network_info)
{
	network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
	
	if (!StartRecvBuffer(network_info))
		return;

	std::string command;
	if (!ParseString(network_info, command))
	{
			fprintf(stdout, "Failed parsing command\n");
			return;
	}

	if (EQUAL_STR == command.compare("STATUS"))
	{
#ifdef DEBUG
		fprintf(stdout, "Received STATUS\n");
#endif
		std::string result_string;
		if (g_max_plaintext.empty())
		{
			result_string = "No results received yet\n";
		}
		else
		{
			ResultString(result_string);
		}

		network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
		sprintf(network_info.buf(), "REMAINING %d\n"\
		                            "BEST ",
		        (int)(g_pending_packets.size()));
		network_info.m_available_bytes = strlen(network_info.const_buf());
		network_info.m_remaining_bytes = network_info.m_available_bytes+result_string.length();
		if (!StartSendBuffer(network_info)) return;
		network_info.SetBuffer(result_string.c_str(), result_string.length());
		network_info.m_available_bytes = result_string.length();
		if (!ContinueSendBuffer(network_info) || 0!=network_info.m_remaining_bytes) return;

#ifdef DEBUG
		fprintf(stdout, "Handled STATUS\n");
#endif
	}
	else if (EQUAL_STR == command.compare("NEW"))
	{
#ifdef DEBUG
		fprintf(stdout, "Received NEW\n");
#endif

		if (0 < network_info.m_remaining_bytes)
		{
			fprintf(stdout, "Unexpected content after NEW\n");
			return;
		}

		network_info.m_remaining_bytes = 6+g_words.length();
		network_info.SetBuffer("WORDS ",6);
		network_info.m_available_bytes = 6;
		if (!StartSendBuffer(network_info)) return;

		network_info.SetBuffer(g_words.c_str(), g_words.length());
		network_info.m_available_bytes = g_words.length();
		if (!ContinueSendBuffer(network_info) || 0!=network_info.m_remaining_bytes) return;
		
		network_info.m_remaining_bytes = 5+g_encrypted_text.length();
		network_info.SetBuffer("TEXT ",5);
		network_info.m_available_bytes = 5;
		if (!StartSendBuffer(network_info)) return;

		network_info.SetBuffer(g_encrypted_text.c_str(), g_encrypted_text.length());
		network_info.m_available_bytes = g_encrypted_text.length();
		if (!ContinueSendBuffer(network_info) || 0!=network_info.m_remaining_bytes) return;

		SendPacket(network_info);

#ifdef DEBUG
		fprintf(stdout, "Handled NEW\n");
#endif
	}
	else if (EQUAL_STR == command.compare("DONE")) //"DONE <setting> <score> <ring/key-settings> <plugboard> <plaintext>"
	{
#ifdef DEBUG
		fprintf(stdout, "Received DONE\n");
#endif

		network_info.m_parsed_pos = 5;
		uint32_t reflector_and_rings_settings;
		if (!ParseInt(network_info, reflector_and_rings_settings))
		{
			fprintf(stdout, "Error parsing packet number\n");
			return;
		}

		PacketInfo received_packet;
		received_packet.FromInt(reflector_and_rings_settings);
		std::string received_packet_str;
		received_packet.ToString(received_packet_str);
		fprintf(stdout, "Received packet %s\n", received_packet_str.c_str());

		RemovePendingPacketInfo(reflector_and_rings_settings);
		
		uint32_t client_score;
		if (!ParseInt(network_info, client_score))
		{
			fprintf(stdout, "Error parsing score\n");
			return;
		}
#ifdef DEBUG
		fprintf(stdout, "Got score %d\n", client_score);
#endif
		if (g_max_score < client_score)
		{
			g_max_score = client_score;
			g_max_reflector_and_ring_settings = reflector_and_rings_settings;
			if (!ParseInt(network_info, g_max_ring_key_settings) ||
			    !ParseString(network_info, g_max_plugboard) ||
			    !ParseString(network_info, g_max_plaintext))
			{
				fprintf(stdout, "Error parsing new highscore\n");
			}
			else
			{
				PrintResult();
			}
		}

		if (0 < network_info.m_remaining_bytes)
		{
			fprintf(stdout, "Unexpected content after DONE plaintext\n");
			return;
		}

		SendPacket(network_info);
#ifdef DEBUG
		fprintf(stdout, "Handled DONE\n");
#endif
	}
	else
	{
		fprintf(stdout, "Unexpected command \"%s\"\n", command.c_str());
	}
}

void MainLoop(int socket_fd)
{
	fprintf(stdout, "Prepared %d packets. Waiting for clients..\n", (int)g_pending_packets.size());

	NetworkInfo network_info;
	while (true) //Wait for STATUS even after all packets are completed
	{
		//Wait for content/connections
		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof their_addr;
		network_info.m_socket_fd = accept(socket_fd, (struct sockaddr*)&their_addr, &addr_size);
#ifdef DEBUG
		fprintf(stdout, "Accepted socket %d\n", network_info.m_socket_fd);
#endif
		HandleClient(network_info);
		close(network_info.m_socket_fd);
#ifdef DEBUG
		fprintf(stderr, "Closed socket %d\n", network_info.m_socket_fd);
#endif
	}
}

void GeneratePendingPackets(bool enabledReflectors[REFLECTOR_COUNT], bool enabledRings[ROTOR_COUNT][RING_COUNT])
{
	uint8_t reflector;
	uint8_t rings[ROTOR_COUNT];
	for (reflector=0; REFLECTOR_COUNT>reflector; reflector++)
	{
		if (!enabledReflectors[reflector]) continue;
		for (rings[LEFT]=0; RING_COUNT>rings[LEFT]; rings[LEFT]++)
		{
			if (!enabledRings[LEFT][rings[LEFT]]) continue;
			for (rings[MIDDLE]=0; RING_COUNT>rings[MIDDLE]; rings[MIDDLE]++)
			{
				if (!enabledRings[MIDDLE][rings[MIDDLE]] || rings[LEFT]==rings[MIDDLE]) continue;
				for (rings[RIGHT]=0; RING_COUNT>rings[RIGHT]; rings[RIGHT]++)
				{
					if (!enabledRings[RIGHT][rings[RIGHT]] || rings[LEFT]==rings[RIGHT] || rings[MIDDLE]==rings[RIGHT]) continue;

					PacketInfo packet_info;
					packet_info.m_reflector = reflector;
					packet_info.m_rings[LEFT] = rings[LEFT];
					packet_info.m_rings[MIDDLE] = rings[MIDDLE];
					packet_info.m_rings[RIGHT] = rings[RIGHT];
					packet_info.m_packet_number = packet_info.ToInt();
					g_pending_packets.push_back(packet_info);
				}
			}
		}
	}
}

void ParseReflectors(const char* reflectors, const char* rings, bool* enabled)
{
	size_t i, j, k;
	for (i=0; REFLECTOR_COUNT>i; i++) enabled[i]=false;

	i=0;
	char ch_reflector, ch_ring;
	while (0 != (ch_reflector=*(reflectors+i++)))
	{
		if ('A'==ch_reflector)
		{
			enabled[0] = true;
		}
		else if ('B'==ch_reflector)
		{
			enabled[1] = true;
		}
		else if ('C'==ch_reflector)
		{
			enabled[1 + M4_THIN_RING_COUNT*CHAR_COUNT + CHAR_COUNT] = true; //Skip entire UKW-b and UKW-c/Beta
		}
		else if ('b'==ch_reflector || 'c'==ch_reflector)
		{
			j = 0;
			while (0 != (ch_ring=*(rings+j++)))
			{
				if ('B'==ch_ring || 'G'==ch_ring)
				{
					for (k=0; CHAR_COUNT>k; k++)
					{
						enabled[1 + (ch_reflector-'b')*M4_THIN_RING_COUNT*CHAR_COUNT + ('B'==ch_ring?0:CHAR_COUNT) + k] = true;
					}
				}
			}
		}
	}
}

void ParseParameter(const char* param, const char* legal_values, bool* enabled)
{
	size_t i = 0;
	while (legal_values[i])	enabled[i++]=false;

	size_t j=0;
	for (; param[j]; j++)
	{
		i = 0;
		for (; legal_values[i]; i++)
		{
			if (param[j] == legal_values[i])
			{
				enabled[i] = true;
				break;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	bool is_m4 = (2<argc && (strchr(argv[2],'B') ||strchr(argv[2],'G')));
	
	if ((5+(is_m4?1:0)) >= argc)
	{
		PrintUsage();
		return -1;
	}

	bool enabledReflectors[REFLECTOR_COUNT];
	bool enabledRings[ROTOR_COUNT][RING_COUNT];
	int param = 1;
	if (is_m4)
	{
		ParseReflectors(argv[param], argv[param+1], enabledReflectors);
		param += 2;
	}
	else
	{
		ParseParameter(argv[param++], "ABC", enabledReflectors);
	}
	ParseParameter(argv[param++], "12345678", enabledRings[LEFT]);
	ParseParameter(argv[param++], "12345678", enabledRings[MIDDLE]);
	ParseParameter(argv[param++], "12345678", enabledRings[RIGHT]);
	GeneratePendingPackets(enabledReflectors, enabledRings);

	if (!ReadWordlist(argv[param++]))
	{
		fprintf(stderr, "Reading wordlist failed\n");
		return -1;
	}

	if (!ReadEncryptedText(argv[param++]))
	{
		fprintf(stderr, "Reading encrypted text failed\n");
		return -1;
	}

	int socket = CreateSocket(param < argc ? argv[param++] : DEFAULT_PORT);
	if (-1 == socket)
	{
		fprintf(stderr, "Initializing server failed\n");
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "Created socket %d\n", socket);
#endif

	g_network_buffer = new char[NETWORK_BUFFER_LENGTH+1];
	if (!g_network_buffer)
	{
		fprintf(stderr, "Allocating network buffer failed\n");
		close(socket);
#ifdef DEBUG
		fprintf(stderr, "Closed socket %d\n", socket);
#endif
		return -1;
	}
	g_network_buffer[NETWORK_BUFFER_LENGTH] = 0;
	
	MainLoop(socket);
	close(socket);
#ifdef DEBUG
	fprintf(stderr, "Closed socket %d\n", socket);
#endif	
	delete[] g_network_buffer;

	PrintResult();

	return 0;
}
