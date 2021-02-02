#pragma once
#include <stdint.h>
#include "GlobalState.h"


void RngInit(const uint8_t starting) {
	gs.rngSeed = starting;
}

uint8_t Rng8(void) {

	++gs.rngSeed; //make GCC happy adding it outhere

	return gs.spriteTimer
		+ RNG_TABLE[gs.rngSeed]
		+ gs.rngSeed
		+ TRUST_ANGLE[gs.rngSeed]
		+ BRAKES_FRICTION[gs.spriteTimer]
		+ AIR_FRICTION[gs.rngSeed]
		+ GROUND_FRICTION[gs.spriteTimer]
		+ gs.mapIndex;
}

uint16_t Rng16(void) {
	return ((uint16_t)Rng8() << 8) | Rng8();
}

uint8_t RngMasked8(const uint8_t maskOffset) {
	return Rng8() & ((1 << maskOffset) - 1);
}

uint16_t RngMasked16(const uint8_t maskOffset) {
	return Rng16() & (((uint16_t)1 << maskOffset) - 1);
}
