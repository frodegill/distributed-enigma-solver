#ifndef _PLUGBOARD_H_
#define _PLUGBOARD_H_

/*
 *  enigma-solver-common
 *
 *  Created by Frode Roxrud Gill, 2015.
 *  This code is GPLv3.
 */

#include "common.h"

#define MAX_SWAP_COUNT (6)
#define MAX_SWAP_HISTORY_STACK_COUNT (6)

#define MORE    (true)
#define NO_MORE (false)

#define REVERT_NONE            (0)
#define REVERT_P1              (1<<0)
#define REVERT_P2              (1<<1)
#define REVERT_P1_AND_P2       (1<<2)
#define INCREMENT_AND_CONTINUE (1<<3)


class Plugboard
{
public:
	void Initialize();
	bool InitializeToNextPlug(const Plugboard& src);
	bool SwapNext();
	void operator=(const Plugboard& src);
	void Push();
	void Pop();
	inline void RevertSwapHistoryStack();
	void Reset();
	void ToString(std::string& str, bool include_compressed) const;

	bool Swap(int8_t c1, int8_t c2, bool add_to_history_stack);

	const uint8_t* GetPlugs() const {return m_plugboard;}

private:
	inline bool SkipRedundantPlugSettings();
	inline bool IncrementAndSkipRedundantPlugSettings();

private:
	uint8_t m_plugboard[CHAR_COUNT];
	uint8_t m_plugboard_backup[CHAR_COUNT];
	uint8_t m_swapchar[MAX_SWAP_COUNT];
	uint8_t m_swap_bflag;
	uint8_t m_swap_history_stack[MAX_SWAP_HISTORY_STACK_COUNT];
	uint8_t m_swap_history_count;
};

#endif // _PLUGBOARD_H_
