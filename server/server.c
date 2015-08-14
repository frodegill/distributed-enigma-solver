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
#define MAX_CALC_TIME_SEC (60*60)

char* g_network_buffer;

std::string g_words;
std::string g_encrypted_text;

int g_max_score = 0;
int g_max_ring_key_settings = 0;
std::string g_max_plugboard;
std::string g_max_plaintext;


struct PendingPacketInfo
{
	int m_reflector_and_rings_settings;
	time_t m_start_time;
};
std::list<PendingPacketInfo> g_pending_packets;


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
			fprintf(stderr, "Closeded socket %d\n", socket_fd);
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

int FindPendingPacketInfo()
{
	time_t find_time = time(NULL) - MAX_CALC_TIME_SEC;
	for (std::list<PendingPacketInfo>::iterator iter = g_pending_packets.begin(); iter != g_pending_packets.end(); ++iter)
	{
		PendingPacketInfo packet_info = *iter;
		if (find_time > packet_info.m_start_time)
		{
			return packet_info.m_reflector_and_rings_settings;
		}
	}
	return NOT_FOUND;
}

void RemovePendingPacketInfo(int reflector_and_rings_settings)
{
	for (std::list<PendingPacketInfo>::iterator iter = g_pending_packets.begin(); iter != g_pending_packets.end(); ++iter)
	{
		PendingPacketInfo pending_packet_info = *iter;
		if (reflector_and_rings_settings == pending_packet_info.m_reflector_and_rings_settings)
		{
			g_pending_packets.erase(iter);
			break;
		}
	}
}

void RegisterPendingPacketInfo(int reflector_and_rings_settings)
{
	time_t current_time = time(NULL);
	for (std::list<PendingPacketInfo>::iterator iter = g_pending_packets.begin(); iter != g_pending_packets.end(); ++iter)
	{
		PendingPacketInfo pending_packet_info = *iter;
		if (reflector_and_rings_settings == pending_packet_info.m_reflector_and_rings_settings)
		{
			pending_packet_info.m_start_time = current_time;
			return;
		}
	}

	PendingPacketInfo pending_packet_info;
	pending_packet_info.m_reflector_and_rings_settings = reflector_and_rings_settings;
	pending_packet_info.m_start_time = current_time;
	g_pending_packets.push_back(pending_packet_info);
}

void SendPacket(PacketInfo& packet, NetworkInfo& network_info)
{
	int reflector_and_rings_settings = FindPendingPacketInfo();
	if (NOT_FOUND == reflector_and_rings_settings)
	{
		packet.Increment();
		reflector_and_rings_settings = packet.ToInt();
	}

	if (PACKET_COUNT <= packet.m_packet_number)
	{
		network_info.SetBuffer("DONE",4);
		network_info.m_available_bytes = network_info.m_remaining_bytes = 4;
		StartSendBuffer(network_info);
	}
	else
	{
		network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
		sprintf(network_info.buf(), "SETTING %d", reflector_and_rings_settings);
		network_info.m_available_bytes = network_info.m_remaining_bytes = strlen(network_info.const_buf());
		if (!StartSendBuffer(network_info) || 0!=network_info.m_remaining_bytes)
			return;

		RegisterPendingPacketInfo(reflector_and_rings_settings);
	}
}

void HandleClient(PacketInfo& packet_info, NetworkInfo& network_info)
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

	if (EQUAL == command.compare("STATUS"))
	{
		fprintf(stdout, "Received STATUS\n");

		network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
		sprintf(network_info.buf(), "PROGRESS %d/%d\n"\
		                            "CLIENTS %d\n"\
		                            "MAX_SCORE %d\n"\
		                            "MAX_RING_KEY %d\n"\
		                            "MAX_PLUGBOARD %s\n"\
		                            "MAX_PLAINTEXT ",
		        packet_info.m_packet_number, PACKET_COUNT,
		        (int)(g_pending_packets.size()),
		        g_max_score,
		        g_max_ring_key_settings,
		        g_max_plugboard.c_str());
		network_info.m_available_bytes = strlen(network_info.const_buf());
		network_info.m_remaining_bytes = network_info.m_available_bytes+g_max_plaintext.length();
		if (!StartSendBuffer(network_info)) return;
		network_info.SetBuffer(g_max_plaintext.c_str(), g_max_plaintext.length());
		if (!ContinueSendBuffer(network_info) || 0!=network_info.m_remaining_bytes) return;

		fprintf(stdout, "Handled STATUS\n");
	}
	else if (EQUAL == command.compare("NEW"))
	{
		fprintf(stdout, "Received NEW\n");

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

		SendPacket(packet_info, network_info);

		fprintf(stdout, "Handled NEW\n");
	}
	else if (EQUAL == command.compare("DONE")) //"DONE <setting> <score> <ring/key-settings> <plugboard> <plaintext>"
	{
		fprintf(stdout, "Received DONE\n");

		network_info.m_parsed_pos = 5;
		int reflector_and_rings_settings;
		if (!ParseInt(network_info, reflector_and_rings_settings))
		{
			fprintf(stdout, "Error parsing new highscore1\n");
			return;
		}
		fprintf(stdout, "Got reflector_ring_setting %d\n", reflector_and_rings_settings);
		RemovePendingPacketInfo(reflector_and_rings_settings);
		
		int client_score;
		if (!ParseInt(network_info, client_score))
		{
			fprintf(stdout, "Error parsing new highscore2\n");
			return;
		}
		fprintf(stdout, "Got score %d\n", client_score);
		if (g_max_score < client_score)
		{
			g_max_score = client_score;
			if (!ParseInt(network_info, g_max_ring_key_settings) ||
			    !ParseString(network_info, g_max_plugboard) ||
			    !ParseString(network_info, g_max_plaintext))
			{
				fprintf(stdout, "Error parsing new highscore3\n");
			}
			else
			{
				fprintf(stdout, "New high: %d %d %s %s\n", g_max_score, g_max_ring_key_settings, g_max_plugboard.c_str(), g_max_plaintext.c_str());
			}
		}

		if (0 < network_info.m_remaining_bytes)
		{
			fprintf(stdout, "Unexpected content after DONE plaintext\n");
			return;
		}

		SendPacket(packet_info, network_info);
		fprintf(stdout, "Handled DONE\n");
	}
	else
	{
		fprintf(stdout, "Unexpected command \"%s\"\n", command.c_str());
	}
}

void MainLoop(int socket_fd)
{
	PacketInfo packet_info;
	packet_info.FromInt(0);
	NetworkInfo network_info;
	while (PACKET_COUNT > packet_info.m_packet_number)
	{
		//Wait for content/connections
		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof their_addr;
		fprintf(stdout, "Waiting...(%d/%d)\n", packet_info.m_packet_number, PACKET_COUNT);

		network_info.m_socket_fd = accept(socket_fd, (struct sockaddr*)&their_addr, &addr_size);
		fprintf(stdout, "Accepted socket %d\n", network_info.m_socket_fd);
		HandleClient(packet_info, network_info);
		close(network_info.m_socket_fd);
		fprintf(stderr, "Closed socket %d\n", network_info.m_socket_fd);
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
	close(socket);
	fprintf(stderr, "Closeded socket %d\n", socket);
	
	delete[] g_network_buffer;

	return 0;
}
