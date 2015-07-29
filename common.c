/*
 *  enigma-solver-sommon
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"


PacketInfo::PacketInfo(int packet_number)
{
	m_packet_number = packet_number;
	m_reflector = packet_number/(RING_COUNT*RING_COUNT*RING_COUNT); packet_number-=m_reflector*RING_COUNT*RING_COUNT*RING_COUNT;
	m_rings[LEFT] = packet_number/(RING_COUNT*RING_COUNT); packet_number-=m_rings[LEFT]*RING_COUNT*RING_COUNT;
	m_rings[MIDDLE] = packet_number/RING_COUNT; packet_number-=m_rings[MIDDLE]*RING_COUNT;
	m_rings[RIGHT] = packet_number;
}

int PacketInfo::ToInt() const {
	return m_reflector*RING_COUNT*RING_COUNT*RING_COUNT+
	       m_rings[LEFT]*RING_COUNT*RING_COUNT+
	       m_rings[MIDDLE]*RING_COUNT+
	       m_rings[RIGHT];
}

void PacketInfo::Increment()
{
	m_packet_number++;
	
	bool invalid = true;
	while (invalid)
	{
		if (RING_COUNT <= ++m_rings[RIGHT])
		{
			m_rings[RIGHT] = 0;
			if (RING_COUNT <= ++m_rings[MIDDLE])
			{
				m_rings[MIDDLE] = 0;
				if (RING_COUNT <= ++m_rings[LEFT])
				{
					m_rings[MIDDLE] = 0;
					m_reflector++;
				}
			}
		}
		invalid = (m_rings[LEFT]==m_rings[MIDDLE] || m_rings[MIDDLE]==m_rings[RIGHT] || m_rings[LEFT]==m_rings[RIGHT]);
	}
}


bool SendBuffer(int socket_fd, const char* buffer, size_t buffer_length)
{
	int bytes_sent;
	while (0<buffer_length)
	{
		if (-1 == (bytes_sent=send(socket_fd, buffer, buffer_length, DEFAULT_SEND_FLAGS)))
			return false;

		buffer += bytes_sent;
		buffer_length -= bytes_sent;
	}
	return true;
}

void SkipCharacter(int socket_fd, char ch, char*& network_buffer, size_t& buffer_pos)
{
	while (true)
	{
		if (0 == network_buffer[buffer_pos])
		{
			int received_bytes = recv(socket_fd, network_buffer, NETWORK_BUFFER_LENGTH, DEFAULT_RECV_FLAGS);
			if (-1 == received_bytes) return;
			
			network_buffer[received_bytes] = 0;
			buffer_pos = 0;
		}

		if (ch != network_buffer[buffer_pos])
			return;

		buffer_pos++;
	}
}

bool ParseInt(int socket_fd, char*& network_buffer, size_t& buffer_pos, int& result)
{
	SkipCharacter(socket_fd, ' ', network_buffer, buffer_pos);

	bool found_result = false;
	result = 0;
	while (true)
	{
		if (0 == network_buffer[buffer_pos])
		{
			int received_bytes = recv(socket_fd, network_buffer, NETWORK_BUFFER_LENGTH, DEFAULT_RECV_FLAGS);
			if (-1 == received_bytes) break;
			
			network_buffer[received_bytes] = 0;
			buffer_pos = 0;
		}

		if ('0'>network_buffer[buffer_pos] || '9'<network_buffer[buffer_pos]) break;

		found_result = true;
		result = result*10 + network_buffer[buffer_pos++]-'0';
	}
	return found_result;
}

bool ParseString(int socket_fd, char*& network_buffer, size_t& buffer_pos, std::string& result)
{
	SkipCharacter(socket_fd, ' ', network_buffer, buffer_pos);

	bool found_result = false;
	result.clear();
	while (true)
	{
		if (0 == network_buffer[buffer_pos])
		{
			int received_bytes = recv(socket_fd, network_buffer, NETWORK_BUFFER_LENGTH, DEFAULT_RECV_FLAGS);
			if (-1 == received_bytes) break;
			
			network_buffer[received_bytes] = 0;
			buffer_pos = 0;
		}

		if ('A'>network_buffer[buffer_pos] || 'Z'<network_buffer[buffer_pos]) break;

		found_result = true;
		result.append(1, network_buffer[buffer_pos++]);
	}
	return found_result;
}
