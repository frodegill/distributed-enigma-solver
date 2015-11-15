#ifndef _KEYSETTING_H_
#define _KEYSETTING_H_

/*
 *  enigma-solver-common
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"
#include "ringsetting.h"


class KeySetting
{
public:
	KeySetting(const uint32_t* ring_turnover_positions, const uint8_t* rings);

	void InitializeStartPosition();
	void InitializeStartPosition(uint8_t left, uint8_t middle, uint8_t right);
	void InitializeStartPosition(const RingSetting& ring_setting);
	bool IncrementStartPosition();
	bool StepRotors();
	void operator=(const KeySetting& src);
	void Push();
	void Pop();
	const uint8_t* GetSettings() const {return m_setting;}
	void CopySettings(const uint8_t* settings);
	void ToString(std::string& str) const;
	uint32_t ToInt();
	uint32_t ToInt(const KeySetting& ring_setting); //Join ring and key setting into one uint32

private:
	static uint32_t ToInt(const uint8_t* setting_3bytes);

private:
	const uint32_t* m_ring_turnover_positions;
	const uint8_t* m_rings;

	uint8_t m_start_setting[ROTOR_COUNT];
	uint8_t m_setting[ROTOR_COUNT];
	uint8_t m_setting_backup[ROTOR_COUNT];
};

#endif // _KEYSETTING_H_
