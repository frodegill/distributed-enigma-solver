#ifndef _PLUGBOARD_H_
#define _PLUGBOARD_H_

/*
 *  enigma-solver-common
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"


class Plugboard
{
public:
	void Initialize();
	bool SwapNext();
	void operator=(const Plugboard& src);
	void Push();
	void Pop();
	void Reset();
	void Print();

private:
	int m_plugboard[CHAR_COUNT];
	int m_plugboard_backup[CHAR_COUNT];
	int m_c1;
	int m_c2;
	int m_tmp_c;
};

#endif // _PLUGBOARD_H_
