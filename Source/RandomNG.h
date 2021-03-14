#ifndef RANDOMNG_H
#define RANDOMNG_H

void RngInit(const uint8_t starting) {
	gs.rngSeed = starting;
}

uint8_t Rng8(void) {

	++gs.rngSeed; //make GCC happy adding it out here

	return gs.spriteTimer
		+ RNG_TABLE[gs.rngSeed]
		+ gs.rngSeed
		+ TRUST_ANGLE[gs.rngSeed]
		+ BRAKES_FRICTION[gs.spriteTimer]
		+ AIR_FRICTION[gs.rngSeed]
		+ GROUND_FRICTION[gs.spriteTimer]
		+ gs.mapIndex;
}

uint8_t RngMasked8(const uint8_t maskOffset) {
	return Rng8() & ((1 << maskOffset) - 1);
}

#endif