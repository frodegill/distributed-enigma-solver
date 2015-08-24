#ifndef _KEYSETTING_H_
#define _KEYSETTING_H_

/*
 *  enigma-solver-common
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"


class KeySetting
{
public:
	KeySetting(const uint32_t* ring_turnover_positions, const uint8_t* rings);

	void InitializeStartPosition();
	bool IncrementStartPosition();
	bool StepRotors();
	void operator=(const KeySetting& src);
	void ToString(std::string& str) const;

private:
	static uint32_t ToInt(const uint8_t* setting_3bytes);

private:
	const uint32_t* m_ring_turnover_positions;
	const uint8_t* m_rings;

	uint8_t m_start_setting[ROTOR_COUNT];
	uint8_t m_setting[ROTOR_COUNT];
};

#endif // _KEYSETTING_H_
