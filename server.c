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


std::string g_words;
std::string g_encrypted_text;

int g_max_score = 0;
int g_max_ring_key_settings = 0;
std::string g_max_plugboard;
std::string g_max_plaintext;


char* g_network_buffer;


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

void SendPacket(PacketInfo& packet, int client_fd)
{
	int reflector_and_rings_settings = FindPendingPacketInfo();
	if (NOT_FOUND == reflector_and_rings_settings)
	{
		packet.Increment();
		reflector_and_rings_settings = packet.ToInt();
	}

	if (PACKET_COUNT <= packet.m_packet_number)
	{
		SendBuffer(client_fd, "DONE", 4);
	}
	else
	{
		sprintf(g_network_buffer, "SETTING %d\r\n", reflector_and_rings_settings);
		if (!SendBuffer(client_fd, g_network_buffer, strlen(g_network_buffer))) return;
		RegisterPendingPacketInfo(reflector_and_rings_settings);
	}
}

void HandleClient(PacketInfo& packet_info, int client_fd)
{
	int received_bytes = recv(client_fd, g_network_buffer, NETWORK_BUFFER_LENGTH, DEFAULT_RECV_FLAGS);
	if (-1 == received_bytes) return;
	
	g_network_buffer[received_bytes] = 0;
	
	if (EQUAL == strncmp("STATUS", g_network_buffer, 6))
	{
		sprintf(g_network_buffer, "PROGRESS %d/%d\r\n", packet_info.m_packet_number, PACKET_COUNT);
		if (!SendBuffer(client_fd, g_network_buffer, strlen(g_network_buffer))) return;

		int client_count = g_pending_packets.size();
		sprintf(g_network_buffer, "CLIENTS %d\r\n", client_count);
		if (!SendBuffer(client_fd, g_network_buffer, strlen(g_network_buffer))) return;

		sprintf(g_network_buffer, "MAX_SCORE %d\r\n", g_max_score);
		if (!SendBuffer(client_fd, g_network_buffer, strlen(g_network_buffer))) return;

		sprintf(g_network_buffer, "MAX_RING_KEY %d\r\n", g_max_ring_key_settings);
		if (!SendBuffer(client_fd, g_network_buffer, strlen(g_network_buffer))) return;

		if (!SendBuffer(client_fd, "MAX_PLUGBOARD ", 14)) return;
		if (!SendBuffer(client_fd, g_max_plugboard.c_str(), g_max_plugboard.length())) return;
		if (!SendBuffer(client_fd, "\r\n", 2)) return;

		if (!SendBuffer(client_fd, "MAX_PLAINTEXT ", 14)) return;
		if (!SendBuffer(client_fd, g_max_plaintext.c_str(), g_max_plaintext.length())) return;
		if (!SendBuffer(client_fd, "\r\n", 2)) return;
	}
	else if (EQUAL == strncmp("NEW", g_network_buffer, 3))
	{
		if (!SendBuffer(client_fd, "WORDS ", 6)) return;
		if (!SendBuffer(client_fd, g_words.c_str(), g_words.length())) return;
		if (!SendBuffer(client_fd, "\r\n", 2)) return;

		if (!SendBuffer(client_fd, "TEXT ", 5)) return;
		if (!SendBuffer(client_fd, g_encrypted_text.c_str(), g_encrypted_text.length())) return;
		if (!SendBuffer(client_fd, "\r\n", 2)) return;

		SendPacket(packet_info, client_fd);
	}
	else if (EQUAL == strncmp("DONE ", g_network_buffer, 5)) //"DONE <setting> <score> <ring/key-settings> <plugboard> <plaintext>"
	{
		size_t buffer_pos = 5;
		int reflector_and_rings_settings;
		if (!ParseInt(client_fd, g_network_buffer, buffer_pos, reflector_and_rings_settings)) return;
		RemovePendingPacketInfo(reflector_and_rings_settings);
		
		int client_score;
		if (!ParseInt(client_fd, g_network_buffer, buffer_pos, client_score)) return;
		if (g_max_score < client_score)
		{
			g_max_score = client_score;
			ParseInt(client_fd, g_network_buffer, buffer_pos, g_max_ring_key_settings);
			ParseString(client_fd, g_network_buffer, buffer_pos, g_max_plugboard);
			ParseString(client_fd, g_network_buffer, buffer_pos, g_max_plaintext);

			fprintf(stdout, "New high: %d %d %s %s\n", g_max_score, g_max_ring_key_settings, g_max_plugboard.c_str(), g_max_plaintext.c_str());
		}

		SendPacket(packet_info, client_fd);
	}

}

void MainLoop(int socket_fd)
{
	PacketInfo packet_info(0);
	while (PACKET_COUNT > packet_info.m_packet_number)
	{
		//Wait for content/connections
		struct sockaddr_storage their_addr;
		socklen_t addr_size = sizeof their_addr;
		fprintf(stdout, "Waiting...(%d/%d)\n", packet_info.m_packet_number, PACKET_COUNT);

		int client_fd = accept(socket_fd, (struct sockaddr*)&their_addr, &addr_size);
		HandleClient(packet_info, client_fd);
		close(client_fd);
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

	g_network_buffer = new char[NETWORK_BUFFER_LENGTH+1];
	if (!g_network_buffer)
	{
		fprintf(stderr, "Allocating network buffer failed\n");
		close(socket);
		return -1;
	}
	
	MainLoop(socket);
	close(socket);
	
	delete[] g_network_buffer;
	return 0;
}
