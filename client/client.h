#ifndef _CLIENT_H_
#define _CLIENT_H_

/*
 *  enigma-solver-client
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"
//#include "plugboard.h"
//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <cstdio>
//#include <cstring>
//#include <unistd.h>

#define OVERFLOW_PROTECTION (5) //From n-1-1 to n+1+1, to avoid range checking when adjusting for ring- and key-setting
#define OVERFLOW_BASE (2) //Pointer to start of middle block


const char* g_hostname;
const char* g_port;
char* g_network_buffer;

bool g_request_status = false;
bool g_done = false;

int g_bigrams[CHAR_COUNT][CHAR_COUNT];
int g_trigrams[CHAR_COUNT][CHAR_COUNT][CHAR_COUNT];
int g_quadgrams[CHAR_COUNT][CHAR_COUNT][CHAR_COUNT][CHAR_COUNT];

uint8_t* g_encrypted_text = NULL;
size_t g_encrypted_text_length = 0;
uint8_t* g_decrypt_buffer = NULL;
uint8_t* g_precalc_plug_paths = NULL;
PacketInfo* g_reflector_ring_settings = NULL;

uint8_t g_reflector_definitions[REFLECTOR_COUNT][OVERFLOW_PROTECTION*CHAR_COUNT];
uint8_t g_ring_definitions[RING_COUNT][OVERFLOW_PROTECTION*CHAR_COUNT];
uint8_t g_inverse_ring_definitions[RING_COUNT][OVERFLOW_PROTECTION*CHAR_COUNT];
uint32_t g_ring_turnover_positions[RING_COUNT];

uint32_t g_ic_charcount[CHAR_COUNT];
uint32_t* g_ic_score;

#endif // _CLIENT_H_
