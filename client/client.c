/*
 *  enigma-solver-client
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "client.h"
#include "plugboard.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>


void InitializeEnigma()
{
	int overflow, reflector, ring, ch;

	char m4_reflector_defs[M4_REFLECTOR_COUNT][CHAR_COUNT+1] = {{"ENKQAUYWJICOPBLMDXZVFTHRGS"},  // UKW-b
	                                                            {"RDOBJNTKVEHMLFCWZAXGYIPSUQ"}}; // UKW-c

	char thin_ring_defs[M4_THIN_RING_COUNT][CHAR_COUNT+1] = {{"LEYJVCNIXWPBQMDRTAKZGFUHOS"},  // Beta
	                                                         {"FSOKANUERHMBTIYCWLQPZXVGJD"}}; // Gamma
	char inverse_thin_ring_defs[M4_THIN_RING_COUNT][CHAR_COUNT+1] = {{"Inverse Beta              "},
	                                                                 {"Inverse Gamma             "}};

	for (ring=0; ring<M4_THIN_RING_COUNT; ring++) {
		for (ch=0; ch<CHAR_COUNT; ch++) {
			inverse_thin_ring_defs[ring][thin_ring_defs[ring][ch]-'A'] = 'A'+ch;
		}
	}

	//Initialize reflectors
	char reflector_defs[REFLECTOR_COUNT][CHAR_COUNT+1] = {{"EJMZALYXVBWFCRQUONTSPIKHGD"},  // A
	                                                      {"YRUHQSLDPXNGOKMIEBFZCWVJAT"},  // B, subset of UKW-b
	                                                      {"UKW-b/Beta in B position  "},
	                                                      {"UKW-b/Beta in C position  "},
	                                                      {"UKW-b/Beta in D position  "},
	                                                      {"UKW-b/Beta in E position  "},
	                                                      {"UKW-b/Beta in F position  "},
	                                                      {"UKW-b/Beta in G position  "},
	                                                      {"UKW-b/Beta in H position  "},
	                                                      {"UKW-b/Beta in I position  "},
	                                                      {"UKW-b/Beta in J position  "},
	                                                      {"UKW-b/Beta in K position  "},
	                                                      {"UKW-b/Beta in L position  "},
	                                                      {"UKW-b/Beta in M position  "},
	                                                      {"UKW-b/Beta in N position  "},
	                                                      {"UKW-b/Beta in O position  "},
	                                                      {"UKW-b/Beta in P position  "},
	                                                      {"UKW-b/Beta in Q position  "},
	                                                      {"UKW-b/Beta in R position  "},
	                                                      {"UKW-b/Beta in S position  "},
	                                                      {"UKW-b/Beta in T position  "},
	                                                      {"UKW-b/Beta in U position  "},
	                                                      {"UKW-b/Beta in V position  "},
	                                                      {"UKW-b/Beta in W position  "},
	                                                      {"UKW-b/Beta in X position  "},
	                                                      {"UKW-b/Beta in Y position  "},
	                                                      {"UKW-b/Beta in Z position  "},
	                                                      {"UKW-b/Gamma in A position "},
	                                                      {"UKW-b/Gamma in B position "},
	                                                      {"UKW-b/Gamma in C position "},
	                                                      {"UKW-b/Gamma in D position "},
	                                                      {"UKW-b/Gamma in E position "},
	                                                      {"UKW-b/Gamma in F position "},
	                                                      {"UKW-b/Gamma in G position "},
	                                                      {"UKW-b/Gamma in H position "},
	                                                      {"UKW-b/Gamma in I position "},
	                                                      {"UKW-b/Gamma in J position "},
	                                                      {"UKW-b/Gamma in K position "},
	                                                      {"UKW-b/Gamma in L position "},
	                                                      {"UKW-b/Gamma in M position "},
	                                                      {"UKW-b/Gamma in N position "},
	                                                      {"UKW-b/Gamma in O position "},
	                                                      {"UKW-b/Gamma in P position "},
	                                                      {"UKW-b/Gamma in Q position "},
	                                                      {"UKW-b/Gamma in R position "},
	                                                      {"UKW-b/Gamma in S position "},
	                                                      {"UKW-b/Gamma in T position "},
	                                                      {"UKW-b/Gamma in U position "},
	                                                      {"UKW-b/Gamma in V position "},
	                                                      {"UKW-b/Gamma in W position "},
	                                                      {"UKW-b/Gamma in X position "},
	                                                      {"UKW-b/Gamma in Y position "},
	                                                      {"UKW-b/Gamma in Z position "},
	                                                      {"UKW-c/Beta in A position  "},
	                                                      {"UKW-c/Beta in B position  "},
	                                                      {"UKW-c/Beta in C position  "},
	                                                      {"UKW-c/Beta in D position  "},
	                                                      {"UKW-c/Beta in E position  "},
	                                                      {"UKW-c/Beta in F position  "},
	                                                      {"UKW-c/Beta in G position  "},
	                                                      {"UKW-c/Beta in H position  "},
	                                                      {"UKW-c/Beta in I position  "},
	                                                      {"UKW-c/Beta in J position  "},
	                                                      {"UKW-c/Beta in K position  "},
	                                                      {"UKW-c/Beta in L position  "},
	                                                      {"UKW-c/Beta in M position  "},
	                                                      {"UKW-c/Beta in N position  "},
	                                                      {"UKW-c/Beta in O position  "},
	                                                      {"UKW-c/Beta in P position  "},
	                                                      {"UKW-c/Beta in Q position  "},
	                                                      {"UKW-c/Beta in R position  "},
	                                                      {"UKW-c/Beta in S position  "},
	                                                      {"UKW-c/Beta in T position  "},
	                                                      {"UKW-c/Beta in U position  "},
	                                                      {"UKW-c/Beta in V position  "},
	                                                      {"UKW-c/Beta in W position  "},
	                                                      {"UKW-c/Beta in X position  "},
	                                                      {"UKW-c/Beta in Y position  "},
	                                                      {"UKW-c/Beta in Z position  "},
	                                                      {"FVPJIAOYEDRZXWGCTKUQSBNMHL"},  // C, subset of UKW-c
	                                                      {"UKW-c/Gamma in B position "},
	                                                      {"UKW-c/Gamma in C position "},
	                                                      {"UKW-c/Gamma in D position "},
	                                                      {"UKW-c/Gamma in E position "},
	                                                      {"UKW-c/Gamma in F position "},
	                                                      {"UKW-c/Gamma in G position "},
	                                                      {"UKW-c/Gamma in H position "},
	                                                      {"UKW-c/Gamma in I position "},
	                                                      {"UKW-c/Gamma in J position "},
	                                                      {"UKW-c/Gamma in K position "},
	                                                      {"UKW-c/Gamma in L position "},
	                                                      {"UKW-c/Gamma in M position "},
	                                                      {"UKW-c/Gamma in N position "},
	                                                      {"UKW-c/Gamma in O position "},
	                                                      {"UKW-c/Gamma in P position "},
	                                                      {"UKW-c/Gamma in Q position "},
	                                                      {"UKW-c/Gamma in R position "},
	                                                      {"UKW-c/Gamma in S position "},
	                                                      {"UKW-c/Gamma in T position "},
	                                                      {"UKW-c/Gamma in U position "},
	                                                      {"UKW-c/Gamma in V position "},
	                                                      {"UKW-c/Gamma in W position "},
	                                                      {"UKW-c/Gamma in X position "},
	                                                      {"UKW-c/Gamma in Y position "},
	                                                      {"UKW-c/Gamma in Z position "}};

	int i, tmp_c;
	for (reflector=0; reflector<M4_REFLECTOR_COUNT; reflector++) {
		for (ring=0; ring<M4_THIN_RING_COUNT; ring++) {
			for (i=0; i<CHAR_COUNT; i++) {
				for (ch=0; ch<CHAR_COUNT; ch++) {
					tmp_c = (thin_ring_defs[ring][(26+ch-i)%26]+26+i-'A')%26;
					tmp_c = m4_reflector_defs[reflector][tmp_c] - 'A';
					tmp_c = (inverse_thin_ring_defs[ring][(26+tmp_c-i)%26]+26+i-'A')%26;
					reflector_defs[1 + reflector*M4_THIN_RING_COUNT*CHAR_COUNT + ring*CHAR_COUNT + i][ch] = tmp_c + 'A';
				}
			}
		}
	}

	for (reflector=0; reflector<REFLECTOR_COUNT; reflector++) {
		for (overflow=0; overflow<OVERFLOW_PROTECTION; overflow++) {
			for (ch=0; ch<CHAR_COUNT; ch++) {
				g_reflector_definitions[reflector][overflow*CHAR_COUNT + ch] = reflector_defs[reflector][ch] - 'A';
			}
		}
	}

	//Initialize ring definitions
	char ring_defs[RING_COUNT][CHAR_COUNT+1] = {{"EKMFLGDQVZNTOWYHXUSPAIBRCJ"},  // I
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
#ifdef DEBUG
			fprintf(stderr, "Closed socket %d\n", socket_fd);
#endif
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
	if (!ParseString(network_info, word) || EQUAL_STR!=word.compare("WORDS")) return false;

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
	if (!ParseString(network_info, text) || EQUAL_STR!=text.compare("TEXT")) return false;
	if (!ParseString(network_info, text)) return false;

	fprintf(stdout, "Encrypted text:\n");
	fprintf(stdout, " \"%s\"\n", text.c_str());
	g_encrypted_text_length = text.length();
	g_encrypted_text = new uint8_t[g_encrypted_text_length];
	g_decrypt_buffer = new uint8_t[g_encrypted_text_length];
	g_precalc_plug_paths = new uint8_t[g_encrypted_text_length*CHAR_COUNT];
	g_ic_score = new uint32_t[g_encrypted_text_length+1];

	size_t i;
	for (i=0; i<=g_encrypted_text_length; i++)
		g_ic_score[i] = i*(i-1);

	for (i=0; i<text.length(); i++)
	{
		g_encrypted_text[i] = text[i]-'A';
	}

	return (0 == network_info.m_remaining_bytes);
}

bool ParseSetting(NetworkInfo& network_info)
{
	std::string text;
	uint32_t setting;
	if (!ParseString(network_info, text)) return false;

	if (EQUAL_STR==text.compare("DONE"))
	{
		g_done = true;
		return true;
	}

	if (EQUAL_STR!=text.compare("SETTING")) return false;
	
	if (!ParseInt(network_info, setting)) return false;

	g_reflector_ring_settings->FromInt(setting);

	std::string packet_str;
	g_reflector_ring_settings->ToString(packet_str);
	fprintf(stdout, "Current packet: %s\n", packet_str.c_str());

	return (0 == network_info.m_remaining_bytes);
}

void DecryptedString(std::string& decrypted_text_str, const uint8_t* buffer)
{
	decrypted_text_str.clear();
	size_t i;
	for (i=0; i<g_encrypted_text_length; i++)
	{
		decrypted_text_str += (char)(buffer[i]+'A');
	}
}

void Decrypt(const uint8_t* ring_settings, const KeySetting& key_setting, const Plugboard& plugboard, uint8_t* decrypted_text_buffer)
{
	KeySetting tmp_key_setting = key_setting;
	const uint8_t* key_settings = tmp_key_setting.GetSettings();
	const uint8_t* plugs = plugboard.GetPlugs();

	int8_t ring;
	int8_t ch;
	size_t i;
	for (i=0; i<g_encrypted_text_length; i++)
	{
		tmp_key_setting.StepRotors();

		ch = plugs[g_encrypted_text[i]];

		for (ring=RIGHT; ring>=LEFT; ring--)
		{
			ch = g_ring_definitions[g_reflector_ring_settings->m_rings[ring]]
															[OVERFLOW_BASE*CHAR_COUNT + ch - ring_settings[ring] + key_settings[ring]] - key_settings[ring] + ring_settings[ring];
		}
		
		ch = g_reflector_definitions[g_reflector_ring_settings->m_reflector][OVERFLOW_BASE * CHAR_COUNT + ch];
		
		for (ring=LEFT; ring<=RIGHT; ring++)
		{
			ch = g_inverse_ring_definitions[g_reflector_ring_settings->m_rings[ring]]
																			[OVERFLOW_BASE*CHAR_COUNT + ch - ring_settings[ring] + key_settings[ring]] - key_settings[ring] + ring_settings[ring];
		}

		while (ch < 0) ch += CHAR_COUNT;
		while (ch >= CHAR_COUNT) ch -= CHAR_COUNT;

		decrypted_text_buffer[i] = plugs[ch];
	}
}

void DecryptPrecalcPlugPaths(const Plugboard& plugboard, uint8_t* decrypted_text_buffer)
{
	const uint8_t* plugs = plugboard.GetPlugs();

	size_t i;
	for (i=0; i<g_encrypted_text_length; i++) {
			decrypted_text_buffer[i] = plugs[g_precalc_plug_paths[i*CHAR_COUNT + plugs[g_encrypted_text[i]]]]; //plugboard in -> enigma -> plugboard out
	}
}

uint32_t ICScore(const uint8_t* decrypted_text)
{
	size_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		g_ic_charcount[i] = 0;
	}

	for (i=0; i<g_encrypted_text_length; i++)
	{
		g_ic_charcount[decrypted_text[i]]++;
	}

	uint32_t ic_score = 0;
	for (i=0; i<CHAR_COUNT; i++)
	{
		ic_score += g_ic_score[g_ic_charcount[i]];
	}

	return ic_score;
}

uint32_t NGramScore(const uint8_t* decrypted_text)
{
	uint32_t ngram_score = 0;
	size_t i;
	for (i=0; i<(g_encrypted_text_length-1); i++) {
		ngram_score += g_bigrams[decrypted_text[i]][decrypted_text[i+1]];
	}
	for (i=0; i<(g_encrypted_text_length-2); i++) {
		ngram_score += g_trigrams[decrypted_text[i]][decrypted_text[i+1]][decrypted_text[i+2]];
	}
	for (i=0; i<(g_encrypted_text_length-3); i++) {
		ngram_score += g_quadgrams[decrypted_text[i]][decrypted_text[i+1]][decrypted_text[i+2]][decrypted_text[i+3]];
	}
	return ngram_score;
}

bool OptimizeScore(Plugboard& plugboard, uint8_t* decrypted_text_buffer, uint32_t& ngram_score)
{
	bool improved = false;

	uint32_t local_ngram_score;
	Plugboard best_plugboard;
	best_plugboard.Initialize();

	plugboard.Reset();
	plugboard.Push();

	Plugboard plugboard2;
	plugboard2.Initialize();
	while (plugboard.SwapNext())
	{
		if (!plugboard2.InitializeToNextPlug(plugboard)) //Avoid swapping plugs we've already tested
			continue;

		do
		{
			DecryptPrecalcPlugPaths(plugboard2, decrypted_text_buffer);

			local_ngram_score = NGramScore(decrypted_text_buffer);
			if (ngram_score<local_ngram_score)
			{
				best_plugboard = plugboard2;
				ngram_score = local_ngram_score;
				improved = true;
			}
		} while (plugboard2.SwapNext());
	}

	if (improved)
	{
		plugboard = best_plugboard;
	}
	else{
		plugboard.Pop();
	}

	return improved;
}

void OptimizeRingSetting(KeySetting& ring_setting, KeySetting& key_setting, const Plugboard& plugboard,
                         uint8_t* decrypted_text_buffer, uint32_t& ngram_score)
{
	ring_setting.Push();
	key_setting.Push();
	uint32_t local_ngram_score = ngram_score;

	size_t step_offset;
	for (step_offset=0; step_offset<CURRENT_OPTIMIZE; step_offset++)
	{
		Decrypt(ring_setting.GetSettings(), key_setting, plugboard, decrypted_text_buffer);

		local_ngram_score = NGramScore(decrypted_text_buffer);
		if (ngram_score<local_ngram_score)
		{
			ring_setting.Push();
			key_setting.Push();
			ngram_score = local_ngram_score;
		}

		ring_setting.StepRotors();
		key_setting.StepRotors();
	}

	ring_setting.Pop();
	key_setting.Pop();
}

void RegisterICScore(uint32_t score, const RingSetting& ring_setting, const KeySetting& key_setting)
{
	size_t i;
	for (i=IC_RESULTS_SIZE-1; i>0 && g_ic_results[i-1].m_score < score; i--)
	{
		g_ic_results[i].m_score = g_ic_results[i-1].m_score;
		*g_ic_results[i].m_ring_setting = *g_ic_results[i-1].m_ring_setting;
		*g_ic_results[i].m_key_setting = *g_ic_results[i-1].m_key_setting;
	}
	g_ic_results[i].m_score = score;
	*g_ic_results[i].m_ring_setting = ring_setting;
	*g_ic_results[i].m_key_setting = key_setting;
}

void FindBestICScores()
{
	size_t i;
	for (i=0; i<IC_RESULTS_SIZE; i++)
	{
		g_ic_results[i].m_score = 0;
	}

	RingSetting ring_setting;
	ring_setting.InitializePosition();
	const uint8_t* ring_settings = ring_setting.GetSettings();

	Plugboard plugboard;
	plugboard.Initialize();

	KeySetting key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
	uint32_t ic_score;
	do
	{
		key_setting.InitializeStartPosition();
		do
		{
			Decrypt(ring_settings, key_setting, plugboard, g_decrypt_buffer);
			ic_score = ICScore(g_decrypt_buffer);
			if (ic_score > g_ic_results[IC_RESULTS_SIZE-1].m_score)
				RegisterICScore(ic_score, ring_setting, key_setting);
		} while (key_setting.IncrementStartPosition());
	} while (ring_setting.IncrementPositionAZZ());
}

void PrecalcPlugPaths(const uint8_t* ring_settings, KeySetting& tmp_key_setting)
{
	const uint8_t* tmp_key_settings = tmp_key_setting.GetSettings();

	size_t encrypted_char_index;
	for (encrypted_char_index=0; encrypted_char_index<g_encrypted_text_length; encrypted_char_index++)
	{
		tmp_key_setting.StepRotors();

		//Precalc all paths for current encryptedCharIndex
		uint8_t precalc_index;
		int8_t ch;
		int8_t ring;
		for (precalc_index=0; precalc_index<CHAR_COUNT; precalc_index++)
		{
			ch = precalc_index;
			for (ring=RIGHT; ring>=LEFT; ring--)
			{
				ch = g_ring_definitions[g_reflector_ring_settings->m_rings[ring]]
																[OVERFLOW_BASE*CHAR_COUNT + ch - ring_settings[ring] + tmp_key_settings[ring]] - tmp_key_settings[ring] + ring_settings[ring];
			}
			
			ch = g_reflector_definitions[g_reflector_ring_settings->m_reflector][OVERFLOW_BASE * CHAR_COUNT + ch];
			
			for (ring=LEFT; ring<=RIGHT; ring++)
			{
				ch = g_inverse_ring_definitions[g_reflector_ring_settings->m_rings[ring]]
																				[OVERFLOW_BASE*CHAR_COUNT + ch - ring_settings[ring] + tmp_key_settings[ring]] - tmp_key_settings[ring] + ring_settings[ring];
			}

			while (ch < 0) ch += CHAR_COUNT;
			while (ch >= CHAR_COUNT) ch -= CHAR_COUNT;
			g_precalc_plug_paths[encrypted_char_index*CHAR_COUNT + precalc_index] = ch;
		}
	}
}

void Calculate(KeySetting& best_ring_setting, KeySetting& best_key_setting, Plugboard& best_plugboard, uint32_t& best_optimized_ngram_score)
{
	RingSetting ring_setting;
	ring_setting.InitializePosition();

	KeySetting key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
	KeySetting tmp_key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);

	Plugboard plugboard;

	KeySetting optimized_ring_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings); //Use KeySetting to get the turnover positions
	KeySetting optimized_key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);

	const uint8_t* ring_settings = ring_setting.GetSettings();

	uint32_t local_ngram_score;
	uint32_t best_ngram_score=0;
	do
	{
		key_setting.InitializeStartPosition();
		do
		{
			tmp_key_setting = key_setting;
			PrecalcPlugPaths(ring_settings, tmp_key_setting);

			//Initialize plugboard
			plugboard.Initialize();

			local_ngram_score = 0;
			while (OptimizeScore(plugboard, g_decrypt_buffer, local_ngram_score));

			if (best_ngram_score<local_ngram_score)
			{
				best_ngram_score = local_ngram_score;

				optimized_ring_setting.CopySettings(ring_settings);
				optimized_key_setting = key_setting;
				OptimizeRingSetting(optimized_ring_setting, optimized_key_setting, plugboard,
				                    g_decrypt_buffer, local_ngram_score);
				if (local_ngram_score>best_optimized_ngram_score)
				{
					best_optimized_ngram_score = local_ngram_score;
					best_ring_setting = optimized_ring_setting;
					best_key_setting = optimized_key_setting;
					best_plugboard = plugboard;

					//Debug hiscore output
					std::string reflector_ring_str, ring_setting_str, key_setting_str, optimized_ring_setting_str, optimized_key_setting_str;
					g_reflector_ring_settings->ToString(reflector_ring_str);
					ring_setting.ToString(ring_setting_str);
					key_setting.ToString(key_setting_str);
					optimized_ring_setting.ToString(optimized_ring_setting_str);
					optimized_key_setting.ToString(optimized_key_setting_str);
					std::string plug_str;
					plugboard.ToString(plug_str, true);
					std::string decrypted_text_str;
					Decrypt(optimized_ring_setting.GetSettings(), optimized_key_setting, plugboard, g_decrypt_buffer);
					DecryptedString(decrypted_text_str, g_decrypt_buffer);
					fprintf(stdout, "%s-%s-%s %s-%s score %d using %s : %s\n",
									reflector_ring_str.c_str(), ring_setting_str.c_str(), key_setting_str.c_str(),
									optimized_ring_setting_str.c_str(), optimized_key_setting_str.c_str(),
									best_optimized_ngram_score, plug_str.c_str(), decrypted_text_str.c_str());
				}
			}

			//Debug progress output
			if (0==(key_setting.ToInt()&0x7F))
			{
				std::string reflector_ring_str, ring_setting_str, key_setting_str;
				g_reflector_ring_settings->ToString(reflector_ring_str);
				ring_setting.ToString(ring_setting_str);
				key_setting.ToString(key_setting_str);
				fprintf(stdout, "%s-%s-%s\n", reflector_ring_str.c_str(), ring_setting_str.c_str(), key_setting_str.c_str());
			}

		} while (key_setting.IncrementStartPosition());
	} while (ring_setting.IncrementPositionAAA());
}

void MainLoop(int& socket_fd)
{
	NetworkInfo network_info;
	network_info.m_socket_fd = socket_fd;
	if (g_request_status)
	{
		network_info.SetBuffer("STATUS",6);
		network_info.m_available_bytes = network_info.m_remaining_bytes = 6;
		if (!StartSendBuffer(network_info) || 0!=network_info.m_remaining_bytes)
			return;

		network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
		if (!StartRecvBuffer(network_info))
			return;

		while (0<network_info.LeftToParse())
		{
			std::string str(network_info.const_buf(), network_info.m_available_bytes);
			network_info.m_parsed_pos = network_info.m_available_bytes;
			fprintf(stdout, "%s", str.c_str());

			if (!ContinueRecvBuffer(network_info))
				return;
		}
	}
	else
	{
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

		while (true)
		{
			network_info.SetBuffer(g_network_buffer,NETWORK_BUFFER_LENGTH);
			if (!StartRecvBuffer(network_info) ||
					!ParseSetting(network_info) || 0<network_info.m_remaining_bytes)
			{
				break;
			}

			close(socket_fd);
#ifdef DEBUG
			fprintf(stderr, "Closed socket %d\n", socket_fd);
#endif
			socket_fd = -1;

			if (g_done)
			{
				break;
			}

#if 1
			FindBestICScores();

			uint32_t best_optimized_ngram_score=0;
			KeySetting best_ring_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
			KeySetting best_key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
			Plugboard best_plugboard;

			size_t i;
			for (i=0; i<IC_RESULTS_SIZE; i++)
			{
				const uint8_t* ring_settings = g_ic_results[i].m_ring_setting->GetSettings();

				KeySetting tmp_key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
				tmp_key_setting = *g_ic_results[i].m_key_setting;

				PrecalcPlugPaths(ring_settings, tmp_key_setting);

				//Initialize plugboard
				Plugboard plugboard;
				plugboard.Initialize();

				uint32_t local_ngram_score = 0;
				while (OptimizeScore(plugboard, g_decrypt_buffer, local_ngram_score));

				if (local_ngram_score > best_optimized_ngram_score)
				{
					best_optimized_ngram_score = local_ngram_score;
					best_ring_setting.InitializeStartPosition(*g_ic_results[i].m_ring_setting);
					best_key_setting = *g_ic_results[i].m_key_setting;
					best_plugboard = plugboard;
				}
			}
#else
			//Calculate
			KeySetting best_ring_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
			KeySetting best_key_setting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
			Plugboard best_plugboard;
			uint32_t best_optimized_ngram_score = 0;
			Calculate(best_ring_setting, best_key_setting, best_plugboard, best_optimized_ngram_score);
#endif
			//Create socket, send result, loop and read next packet info
			socket_fd = CreateSocket(g_hostname, g_port);
			if (-1 == socket_fd)
			{
				fprintf(stderr, "Connecting to server failed\n");
				return;
			}
#ifdef DEBUG
			fprintf(stderr, "Created socket %d\n", socket_fd);
#endif

			std::string plugboard_str;
			best_plugboard.ToString(plugboard_str, false);
			
			std::string decrypted_text_str;
			Decrypt(best_ring_setting.GetSettings(), best_key_setting, best_plugboard, g_decrypt_buffer);
			DecryptedString(decrypted_text_str, g_decrypt_buffer);

			sprintf(network_info.buf(), "DONE %d %d %d %s ", g_reflector_ring_settings->ToInt(),
							best_optimized_ngram_score, best_key_setting.ToInt(best_ring_setting), plugboard_str.c_str());
			network_info.m_available_bytes = strlen(network_info.const_buf());
			network_info.m_remaining_bytes = network_info.m_available_bytes+decrypted_text_str.length();
			if (!StartSendBuffer(network_info))
			{
				return;
			}

			network_info.SetBuffer(decrypted_text_str.c_str(), decrypted_text_str.length());
			network_info.m_available_bytes = decrypted_text_str.length();
			if (!ContinueSendBuffer(network_info) || 0!=network_info.m_remaining_bytes)
			{
				return;
			}
		}
	}
}

void PrintUsage()
{
	fprintf(stdout, "\nParameters: <server hostname> [server port] (default port: 2720) [--status]\n\n"
									"Example: ./enigma-solver-client localhost 2720\n\n");
}

int main(int argc, char* argv[])
{
	if (EQUAL_STR==strcmp(argv[argc-1], "--status"))
	{
		g_request_status = true;
		argc--;
	}

	if (1 >= argc)
	{
		PrintUsage();
		return -1;
	}

	InitializeEnigma();

	g_ic_results = new ICResult[IC_RESULTS_SIZE];
	size_t i;
	for (i=0; i<IC_RESULTS_SIZE; i++)
	{
		g_ic_results[i].m_score = 0;
		g_ic_results[i].m_ring_setting = new RingSetting;
		g_ic_results[i].m_ring_setting->InitializePosition();
		g_ic_results[i].m_key_setting = new KeySetting(g_ring_turnover_positions, g_reflector_ring_settings->m_rings);
		g_ic_results[i].m_key_setting->InitializeStartPosition();
	}

	g_hostname = argv[1];
	g_port = (3 <= argc ? argv[2] : DEFAULT_PORT);
	int socket = CreateSocket(g_hostname, g_port);
	if (-1 == socket)
	{
		fprintf(stderr, "Connecting to server failed\n");
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
		fprintf(stderr, "Closeded socket %d\n", socket);
#endif
		return -1;
	}
	g_network_buffer[NETWORK_BUFFER_LENGTH] = 0;
	g_reflector_ring_settings = new PacketInfo;

	MainLoop(socket);
	if (-1 != socket)
	{
		close(socket);
#ifdef DEBUG
		fprintf(stderr, "Closed socket %d\n", socket);
#endif
	}

	delete[] g_network_buffer;

	delete g_reflector_ring_settings;
	delete[] g_ic_score;
	delete[] g_ic_results;
	delete[] g_precalc_plug_paths;
	delete[] g_decrypt_buffer;
	delete[] g_encrypted_text;
	return 0;
}
