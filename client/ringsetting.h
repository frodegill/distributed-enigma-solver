#ifndef _RINGSETTING_H_
#define _RINGSETTING_H_

/*
 *  enigma-solver-common
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"


class RingSetting
{
public:
	void InitializePosition();
	bool IncrementPosition();
	void ToString(std::string& str) const;
	const uint8_t* GetSettings() const {return m_setting;}

private:
	uint8_t m_setting[ROTOR_COUNT];
};

#endif // _RINGSETTING_H_
