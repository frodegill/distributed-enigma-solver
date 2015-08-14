#ifndef _COMMON_H_
#define _COMMON_H_

/*
 *  enigma-solver-common
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include <netdb.h>
#include <string>

#define CHAR_COUNT (26)

#define REFLECTOR_COUNT (2) //B and C
#define ROTOR_COUNT (3) //M3
#define RING_COUNT (8) //I, II, III, IV, V, VI, VII and VIII (Navy M3 1939+)
#define PACKET_COUNT (REFLECTOR_COUNT*RING_COUNT*(RING_COUNT-1)*(RING_COUNT-2))
#define LEFT (0)
#define MIDDLE (1)
#define RIGHT (2)

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

#define TIMEOUT (60)

#define NOT_FOUND (-1)
#define EQUAL (0)
#define NETWORK_BUFFER_LENGTH (8*1024)
#define DEFAULT_SEND_FLAGS (0)
#define DEFAULT_RECV_FLAGS (0)

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))


struct NetworkInfo
{
private:
	const char* m_buffer;
	size_t m_buffer_size;

public:
	int m_socket_fd;

	size_t m_remaining_bytes; //Total remaining bytes (may be larger than one network buffer)
	size_t m_available_bytes; //Bytes either read and ready to parse, or written and ready to send

	size_t m_parsed_pos;

	void SetBuffer(const char* buffer, size_t buffer_size) {m_buffer=buffer; m_buffer_size=buffer_size;
	                                                        m_available_bytes=m_parsed_pos=0;}
	const char* const_buf() const {return m_buffer;}
	char* buf() const {return (char*)m_buffer;}
	size_t GetBufferSize() const {return m_buffer_size;}

	size_t LeftToParse() const {return m_available_bytes-m_parsed_pos;}
};

struct PacketInfo
{
	int m_packet_number;
	int m_reflector;
	int m_rings[3];

	void FromInt(int packet_number);
	int ToInt() const;
	void Increment();
};

bool StartSendBuffer(NetworkInfo& network_info);
bool ContinueSendBuffer(NetworkInfo& network_info);

bool StartRecvBuffer(NetworkInfo& network_info);
bool ContinueRecvBuffer(NetworkInfo& network_info);

void SkipCharacter(NetworkInfo& network_info, char ch);
bool ParseInt(NetworkInfo& network_info, int& result);
bool ParseString(NetworkInfo& network_info, std::string& result);

#endif // _COMMON_H_
