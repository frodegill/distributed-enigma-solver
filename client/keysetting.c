/*
 *  enigma-solver-client plugboard
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "keysetting.h"


KeySetting::KeySetting(const uint32_t* ring_turnover_positions, const uint8_t* rings)
: m_ring_turnover_positions(ring_turnover_positions),
  m_rings(rings)
{
}

void KeySetting::InitializeStartPosition()
{
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_start_setting[i] = m_setting[i] = 0;
	}
}

void KeySetting::InitializeStartPosition(const RingSetting& ring_setting)
{
	const uint8_t* settings = ring_setting.GetSettings();
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_start_setting[i] = settings[i];
	}
}

bool KeySetting::IncrementStartPosition()
{
	if (CHAR_COUNT == ++m_start_setting[RIGHT])
	{
		m_start_setting[RIGHT] = 0;
		if (CHAR_COUNT == ++m_start_setting[MIDDLE])
		{
			m_start_setting[MIDDLE] = 0;
			if (CHAR_COUNT == ++m_start_setting[LEFT])
			{
				m_start_setting[LEFT] = 0;
				return false;
			}
		}
	}
	return true;
}

bool KeySetting::StepRotors()
{
	uint32_t start_value = ToInt(m_setting);

	bool middle_ring_on_turnover_position = (m_ring_turnover_positions[m_rings[MIDDLE]] & (1<<m_setting[MIDDLE])) != 0;
	if ((m_ring_turnover_positions[m_rings[RIGHT]] & (1<<m_setting[RIGHT])) != 0 || //Normal turnover right->middle ring
					middle_ring_on_turnover_position) //Double-step
	{
			if (middle_ring_on_turnover_position) //Normal turnover middle->left ring
			{
					m_setting[LEFT]++;
					if (m_setting[LEFT] >= CHAR_COUNT)
							m_setting[LEFT] -= CHAR_COUNT;
			}
			m_setting[MIDDLE]++;
			if (m_setting[MIDDLE] >= CHAR_COUNT)
					m_setting[MIDDLE] -= CHAR_COUNT;
	}
	m_setting[RIGHT]++;
	if (m_setting[RIGHT] >= CHAR_COUNT)
			m_setting[RIGHT] -= CHAR_COUNT;

	uint32_t end_value = ToInt(m_setting);
	uint32_t bailout_value = ToInt(m_start_setting);
	return (start_value<bailout_value && end_value>=bailout_value);
}

void KeySetting::operator=(const KeySetting& src)
{
	m_ring_turnover_positions = src.m_ring_turnover_positions;
	m_rings = src.m_rings;

	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_start_setting[i] = src.m_start_setting[i];
		m_setting[i] = src.m_setting[i];
	}
}

void KeySetting::Push()
{
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_setting_backup[i] = m_setting[i];
	}
}

void KeySetting::Pop()
{
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_setting[i] = m_setting_backup[i];
	}
}

void KeySetting::CopySettings(const uint8_t* settings)
{
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_setting[i] = settings[i];
	}
}

void KeySetting::ToString(std::string& str) const
{
	str.empty();
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		str += (char)(m_setting[i]+'A');
	}
}

uint32_t KeySetting::ToInt(const KeySetting& ring_setting)
{
	return ring_setting.m_setting[LEFT]*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT +
	       ring_setting.m_setting[MIDDLE]*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT +
	       ring_setting.m_setting[RIGHT]*CHAR_COUNT*CHAR_COUNT*CHAR_COUNT +
	       m_setting[LEFT]*CHAR_COUNT*CHAR_COUNT +
	       m_setting[MIDDLE]*CHAR_COUNT +
	       m_setting[RIGHT];
}

uint32_t KeySetting::ToInt(const uint8_t* setting_3bytes)
{
	return setting_3bytes[LEFT]<<16|setting_3bytes[MIDDLE]<<8|setting_3bytes[RIGHT];
}
