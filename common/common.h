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

#define M4_THIN_RING_COUNT (2)  //Beta and Gamma
#define M4_REFLECTOR_COUNT (2)  //UKW-b and UKW-c
#define REFLECTOR_COUNT (1 + (M4_REFLECTOR_COUNT*M4_THIN_RING_COUNT*CHAR_COUNT)) //A + UKW-b/UKW-c (UKW-b/Beta and UKW-c/Gamma in first positions are compatible with M3 reflector B and C)
#define ROTOR_COUNT (3) //M3. Extra M4 ring is handled by reflector
#define RING_COUNT (8) //I, II, III, IV, V, VI, VII, VIII (M4 thin rings are handled by reflectors)
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

#define NOT_FOUND ((uint32_t)-1)
#define EQUAL_STR (0)
#define EQUAL (true)
#define NOT_EQUAL (false)
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
	uint32_t m_packet_number;
	uint8_t  m_reflector;
	uint8_t  m_rings[3];

	void FromInt(uint32_t packet_number);
	uint32_t ToInt() const;
	void ToString(std::string& str) const;
	void ReflectorToString(std::string& str) const;
};

bool StartSendBuffer(NetworkInfo& network_info);
bool ContinueSendBuffer(NetworkInfo& network_info);

bool StartRecvBuffer(NetworkInfo& network_info);
bool ContinueRecvBuffer(NetworkInfo& network_info);

void SkipCharacter(NetworkInfo& network_info, char ch);
bool ParseInt(NetworkInfo& network_info, uint32_t& result);
bool ParseString(NetworkInfo& network_info, std::string& result);

#endif // _COMMON_H_
