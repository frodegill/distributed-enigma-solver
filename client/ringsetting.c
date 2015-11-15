/*
 *  enigma-solver-client plugboard
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "ringsetting.h"


void RingSetting::InitializePosition()
{
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		m_setting[i] = 0;
	}
}

bool RingSetting::IncrementPosition()
{
	return (CHAR_COUNT != ++m_setting[RIGHT]); //AAA -> AAZ
//	return false; //AAA -> AAA
}

void RingSetting::ToString(std::string& str) const
{
	str.empty();
	uint8_t i;
	for (i=0; i<ROTOR_COUNT; i++)
	{
		str += (char)(m_setting[i]+'A');
	}
}
