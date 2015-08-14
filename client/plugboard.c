/*
 *  enigma-solver-client plugboard
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "plugboard.h"
#include <cstdio>


void Plugboard::Initialize()
{
	int i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = i;
	}
	Push();
	Reset();
}

bool Plugboard::SwapNext()
{
	m_tmp_c = m_plugboard[m_c1];
	m_plugboard[m_c1] = m_plugboard[m_c2];
	m_plugboard[m_c2] = m_tmp_c;

	do
	{
		if (CHAR_COUNT<=++m_c2)
		{
			if (CHAR_COUNT<=++m_c1)
			{
				return false;
			}
			m_c2=0;
		}
	} while (m_c1>=m_c2);

	m_tmp_c = m_plugboard[m_c1];
	m_plugboard[m_c1] = m_plugboard[m_c2];
	m_plugboard[m_c2] = m_tmp_c;
	return true;
}

void Plugboard::operator=(const Plugboard& src)
{
	int i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = src.m_plugboard[i];
	}
}

void Plugboard::Push()
{
	int i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard_backup[i] = m_plugboard[i];
	}
}

void Plugboard::Pop()
{
	int i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = m_plugboard_backup[i];
	}
}

void Plugboard::Reset()
{
	m_c1 = m_c2 = 0;
}

void Plugboard::Print()
{
	std::string s;
	int i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		s.append(1, (char)(m_plugboard[i]+'A'));
	}
	fprintf(stdout, "%s\n", s.c_str());
}
