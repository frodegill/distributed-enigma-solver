/*
 *  enigma-solver-sommon
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"
#include <cstdio>
#include <string.h>
#include <time.h>


void PacketInfo::FromInt(uint32_t packet_number)
{
	m_packet_number = packet_number;
	m_reflector = packet_number/(RING_COUNT*RING_COUNT*RING_COUNT); packet_number-=m_reflector*RING_COUNT*RING_COUNT*RING_COUNT;
	m_rings[LEFT] = packet_number/(RING_COUNT*RING_COUNT); packet_number-=m_rings[LEFT]*RING_COUNT*RING_COUNT;
	m_rings[MIDDLE] = packet_number/RING_COUNT; packet_number-=m_rings[MIDDLE]*RING_COUNT;
	m_rings[RIGHT] = packet_number;
}

uint32_t PacketInfo::ToInt() const {
	return m_reflector*RING_COUNT*RING_COUNT*RING_COUNT+
	       m_rings[LEFT]*RING_COUNT*RING_COUNT+
	       m_rings[MIDDLE]*RING_COUNT+
	       m_rings[RIGHT];
}

void PacketInfo::ToString(std::string& str) const
{
	str.clear();
	if (0 == ToInt())
	{
		str = "-";
	}
	else
	{
		std::string reflector_str;
		ReflectorToString(reflector_str);
		str += reflector_str;
		uint8_t i;
		for (i=0; i<ROTOR_COUNT; i++)
		{
			str += (char)(m_rings[i]+'1');
		}
	}
}

void PacketInfo::ReflectorToString(std::string& str) const
{
	if (0==m_reflector)
	{
		str = "A";
	}
	else
	{
		uint8_t index = m_reflector-1;
		uint8_t reflector = index/(M4_THIN_RING_COUNT*CHAR_COUNT); index-=reflector*M4_THIN_RING_COUNT*CHAR_COUNT;
		uint8_t ring = index/CHAR_COUNT; index-=ring*CHAR_COUNT;
		if (0==index)
		{
			str = (char)('B'+reflector);
		}
		else
		{
			str = (char)('b'+reflector);
			str += (0==ring?"B":"G");
			str += (char)('A'+index);
		}
	}
}

bool StartSendBuffer(NetworkInfo& network_info)
{
	uint8_t size_buffer[4];
	size_buffer[0] = (network_info.m_remaining_bytes&0xFF000000)>>24;
	size_buffer[1] = (network_info.m_remaining_bytes&0x00FF0000)>>16;
	size_buffer[2] = (network_info.m_remaining_bytes&0x0000FF00)>>8;
	size_buffer[3] = (network_info.m_remaining_bytes&0x000000FF);

	int bytes_sent = 0;
	if (-1 == (bytes_sent=::send(network_info.m_socket_fd, size_buffer, 4, DEFAULT_SEND_FLAGS)) ||
			4 != bytes_sent)
	{
		return false;
	}

	network_info.m_parsed_pos = 0;
	return ContinueSendBuffer(network_info);
}

bool ContinueSendBuffer(NetworkInfo& network_info)
{
	size_t total_bytes_sent = 0;
	int bytes_sent;
	while (total_bytes_sent < network_info.m_available_bytes)
	{
		if (-1 == (bytes_sent=::send(network_info.m_socket_fd,
		                             network_info.const_buf()+total_bytes_sent,
		                             network_info.m_available_bytes-total_bytes_sent,
		                             DEFAULT_SEND_FLAGS)))
		{
			return false;
		}

#ifdef DEBUG
		fprintf(stdout, "Sent %d of %d bytes\n", bytes_sent, (int)(network_info.m_remaining_bytes));
#endif

		total_bytes_sent += bytes_sent;
		network_info.m_remaining_bytes -= bytes_sent;
	}

	return true;
}

bool StartRecvBuffer(NetworkInfo& network_info)
{
	uint8_t size_buffer[4];
	int bytes_received;
	if (-1 == (bytes_received=::recv(network_info.m_socket_fd, size_buffer, 4, DEFAULT_RECV_FLAGS)) ||
	    4 != bytes_received)
	{
		return false;
	}

	network_info.m_available_bytes = 0;
	network_info.m_parsed_pos = 0;
	network_info.m_remaining_bytes = (size_t)(size_buffer[0])<<24 | (size_t)(size_buffer[1])<<16 | (size_t)(size_buffer[2])<<8 | size_buffer[3];
	return ContinueRecvBuffer(network_info);
}

bool ContinueRecvBuffer(NetworkInfo& network_info)
{
	if (0 < network_info.LeftToParse())
	{
		memmove(network_info.buf(), network_info.const_buf()+network_info.m_parsed_pos, network_info.LeftToParse());
		network_info.m_parsed_pos = network_info.LeftToParse();
	}

	time_t start_time = ::time(NULL);
	size_t bytes_to_receive = MIN(network_info.GetBufferSize()-network_info.m_available_bytes,network_info.m_remaining_bytes);
	size_t total_bytes_received = 0;
	int bytes_received;
	while (total_bytes_received < bytes_to_receive)
	{
		if (-1 == (bytes_received=::recv(network_info.m_socket_fd,
		                                 network_info.buf()+network_info.m_available_bytes,
		                                 bytes_to_receive-total_bytes_received,
		                                 DEFAULT_SEND_FLAGS)))
		{
			return false;
		}

#ifdef DEBUG
		fprintf(stdout, "Received %d of %d bytes\n", bytes_received, (int)(network_info.m_remaining_bytes));
#endif

		total_bytes_received += bytes_received;
		network_info.m_available_bytes += bytes_received;
		network_info.m_remaining_bytes -= bytes_received;

		if (0 < network_info.m_remaining_bytes)
		{
			if (TIMEOUT < (time(NULL)-start_time))
				return false;

			//Sleep 1ms
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 1*1000*1000;
#ifdef DEBUG
			fprintf(stdout, "Sleeping 1ms\n");
#endif
			::nanosleep(&ts, NULL);
		}
	}

	return true;
}

void SkipCharacter(NetworkInfo& network_info, char ch)
{
	while (network_info.m_parsed_pos < (network_info.m_available_bytes+network_info.m_remaining_bytes))
	{
		if (network_info.m_parsed_pos >= network_info.m_available_bytes && !ContinueRecvBuffer(network_info))
			return;

		if (ch != *(network_info.const_buf()+network_info.m_parsed_pos))
			return;

		network_info.m_parsed_pos++;
	}
}

bool ParseInt(NetworkInfo& network_info, uint32_t& result)
{
	SkipCharacter(network_info, ' ');

	bool found_result = false;
	result = 0;
	char ch;
	while (network_info.m_parsed_pos < (network_info.m_available_bytes+network_info.m_remaining_bytes))
	{
		if (network_info.m_parsed_pos >= network_info.m_available_bytes && !ContinueRecvBuffer(network_info))
			return false;

		ch = *(network_info.const_buf()+network_info.m_parsed_pos);
		if ('0'>ch || '9'<ch) break;

		found_result = true;
		result = result*10 + ch-'0';
		network_info.m_parsed_pos++;
	}
	return found_result;
}

bool ParseString(NetworkInfo& network_info, std::string& result)
{
	SkipCharacter(network_info, ' ');

	bool found_result = false;
	result.clear();
	char ch;
	while (network_info.m_parsed_pos < (network_info.m_available_bytes+network_info.m_remaining_bytes))
	{
		if (network_info.m_parsed_pos >= network_info.m_available_bytes && !ContinueRecvBuffer(network_info))
			return false;

		ch = *(network_info.const_buf()+network_info.m_parsed_pos);
		if ('A'>ch || 'Z'<ch) break;

		found_result = true;
		result.append(1, ch);
		network_info.m_parsed_pos++;
	}
	return found_result;
}
