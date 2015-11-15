/*
 *  enigma-solver-client plugboard
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "plugboard.h"
#include <cstdio>
#include <stdlib.h>


void Plugboard::Initialize()
{
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = i;
	}
	Push();
	Reset();
}

bool Plugboard::InitializeToNextPlug(const Plugboard& src)
{
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = src.m_plugboard[i];
		m_plugboard_backup[i] = src.m_plugboard_backup[i];
	}

	m_swapchar[0] = src.m_swapchar[0]+1;
	if (m_swapchar[0]==src.m_swapchar[1]) m_swapchar[0]++;

	m_swapchar[1] = m_swapchar[0]+1;
	if (m_swapchar[1]==src.m_swapchar[1]) m_swapchar[1]++;
	
	m_swap_bflag = REVERT_NONE;
	m_swap_history_count = 0;
	return (CHAR_COUNT>m_swapchar[1]);
}

bool Plugboard::SwapNext()
{
	RevertSwapHistoryStack();

	if (CHAR_COUNT <= m_swapchar[0])
		return NO_MORE;

	if (m_swap_bflag&INCREMENT_AND_CONTINUE)
	{
		if (!IncrementAndSkipRedundantPlugSettings())
			return NO_MORE;

		m_swap_bflag &= ~INCREMENT_AND_CONTINUE;
	}
	
	if (REVERT_NONE != m_swap_bflag)
	{
		uint8_t tmp_c0 = m_plugboard[m_swapchar[0]];
		uint8_t tmp_c1 = m_plugboard[m_swapchar[1]];

		if(m_swap_bflag&(REVERT_P1|REVERT_P1_AND_P2))
		{
			Swap(m_swapchar[0], tmp_c0, true);
		}
		if(m_swap_bflag&(REVERT_P2|REVERT_P1_AND_P2))
		{
			Swap(m_swapchar[1], tmp_c1, true);
		}

		if (m_swap_bflag&REVERT_P1)
		{
			Swap(tmp_c0, m_swapchar[1], true);
			m_swap_bflag &= ~REVERT_P1;
		}
		else if (m_swap_bflag&REVERT_P2)
		{
			Swap(tmp_c1, m_swapchar[0], true);
			m_swap_bflag &= ~REVERT_P2;
		}
		else //REVERT_P1_AND_P2
		{
			Swap(tmp_c0, tmp_c1, true);
			m_swap_bflag = REVERT_NONE;
		}

		if (REVERT_NONE==m_swap_bflag)
		{
			m_swap_bflag = INCREMENT_AND_CONTINUE;
		}
	}
	else
	{
		if (!(m_swapchar[0]==0 && m_swapchar[1]==0)) //After Reset(), both are 0
		{
			if (!(m_plugboard[m_swapchar[0]]==m_swapchar[1] && m_plugboard[m_swapchar[1]]==m_swapchar[0]))
			{
				if (m_plugboard[m_swapchar[0]] != m_swapchar[0])
					m_swap_bflag|=REVERT_P1;

				if (m_plugboard[m_swapchar[1]] != m_swapchar[1])
					m_swap_bflag|=REVERT_P2;

				if ((REVERT_P1|REVERT_P2)==(m_swap_bflag&(REVERT_P1|REVERT_P2)))
					m_swap_bflag|=REVERT_P1_AND_P2;

				if(m_swap_bflag&(REVERT_P1|REVERT_P1_AND_P2))
					Swap(m_swapchar[0], m_plugboard[m_swapchar[0]], true);

				if(m_swap_bflag&(REVERT_P2|REVERT_P1_AND_P2))
					Swap(m_swapchar[1], m_plugboard[m_swapchar[1]], true);

				Swap(m_swapchar[0], m_swapchar[1], true);

				if (0 != (m_swap_bflag&(REVERT_P1|REVERT_P2|REVERT_P1_AND_P2)))
				{
					return MORE;
				}
			}
		}

		m_swap_bflag |= INCREMENT_AND_CONTINUE;
	}

	return MORE;
}

void Plugboard::operator=(const Plugboard& src)
{
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = src.m_plugboard[i];
		m_plugboard_backup[i] = src.m_plugboard_backup[i];
	}
	for (i=0; i<MAX_SWAP_COUNT; i++)
	{
		m_swapchar[i] = src.m_swapchar[i];
	}
	m_swap_bflag = src.m_swap_bflag;

	m_swap_history_count = src.m_swap_history_count;
#ifdef DEBUG
	if (m_swap_history_count >= MAX_SWAP_HISTORY_STACK_COUNT)
	{
		fprintf(stdout, "m_swap_history_count is %d\n", (int)m_swap_history_count);
	}
#endif
	for (i=0; i<m_swap_history_count; i++)
	{
		m_swap_history_stack[i] = src.m_swap_history_stack[i];
	}
}

void Plugboard::Push()
{
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard_backup[i] = m_plugboard[i];
	}
}

void Plugboard::Pop()
{
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		m_plugboard[i] = m_plugboard_backup[i];
	}
	Reset();
}

void Plugboard::RevertSwapHistoryStack()
{
	if (m_swap_history_count >= MAX_SWAP_HISTORY_STACK_COUNT)
	{
		fprintf(stdout, "m_swap_history_count is %d\n", (int)m_swap_history_count);
	}
	while (1 < m_swap_history_count)
	{
		Swap(m_swap_history_stack[m_swap_history_count-1], m_swap_history_stack[m_swap_history_count-2], false);
		m_swap_history_count -= 2;
		if (m_swap_history_count >= MAX_SWAP_HISTORY_STACK_COUNT)
		{
			fprintf(stdout, "m_swap_history_count is %d\n", (int)m_swap_history_count);
		}
	}
}

void Plugboard::Reset()
{
	m_swap_bflag = REVERT_NONE;
	m_swapchar[0] = m_swapchar[1] = 0;
	m_swap_history_count = 0;
}

void Plugboard::ToString(std::string& str, bool include_compressed) const
{
	str.empty();
	uint8_t i;
	for (i=0; i<CHAR_COUNT; i++)
	{
		str += (char)(m_plugboard[i]+'A');
	}

	if (include_compressed)
	{
		str += " [ ";
		for (i=0; i<CHAR_COUNT; i++)
		{
			if (m_plugboard[i] > i)
			{
				str += (char)(i+'A');
				str += '-';
				str += (char)(m_plugboard[i]+'A');
				str += ' ';
			}
		}
		str += ']';
	}
}

bool Plugboard::Swap(int8_t c1, int8_t c2, bool add_to_history_stack)
{
#ifdef DEBUG
	if (c1==c2)
	{
		fprintf(stdout, "Plug %c swaps to itself\n", (char)(c1+'A'));
		exit(-1);
	}
	if (m_plugboard[c1]!=c1 && m_plugboard[c1]!=c2)
	{
		fprintf(stdout, "Plug %c already swapped\n", (char)(c1+'A'));
		exit(-1);
	}
	if (m_plugboard[c2]!=c2 && m_plugboard[c2]!=c1)
	{
		fprintf(stdout, "Plug %c already swapped\n", (char)(c2+'A'));
		exit(-1);
	}
#endif

	uint8_t tmp_c = m_plugboard[c1];
	m_plugboard[c1] = m_plugboard[c2];
	m_plugboard[c2] = tmp_c;
	if (add_to_history_stack)
	{
#ifdef DEBUG
		if ((m_swap_history_count+2) > MAX_SWAP_COUNT)
		{
			fprintf(stdout, "Max swap history count reached\n");
			exit(-1);
		}
#endif
		m_swap_history_stack[m_swap_history_count++] = c1;
		m_swap_history_stack[m_swap_history_count++] = c2;
	}
	return true;
}

bool Plugboard::SkipRedundantPlugSettings()
{
	while (m_swapchar[0]>=m_swapchar[1]) //Only test where swapchar[0]<swapchar[1]. By mirroring we know the rest is already taken care of
	{
		if (CHAR_COUNT<=++m_swapchar[1])
		{
			if (CHAR_COUNT<=++m_swapchar[0])
			{
				return NO_MORE;
			}
			m_swapchar[1]=0;
		}
	}
	return MORE;
}

bool Plugboard::IncrementAndSkipRedundantPlugSettings()
{
	if (CHAR_COUNT<=++m_swapchar[1])
	{
		if (CHAR_COUNT<=++m_swapchar[0])
		{
			return NO_MORE;
		}
		m_swapchar[1]=0;
	}
	return SkipRedundantPlugSettings();
}
