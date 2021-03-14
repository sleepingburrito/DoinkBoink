#ifndef PARTICLES_H
#define PARTICLES_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "Tools.h"
#include "GlobalState.h"
#include "Graphics.h"



//particles system
void InitParticles(void) {
	ZeroOut((uint8_t*)gs.particles, sizeof(gs.particles));
}

void ParticleAdd(
	const uint16_t x,
	const uint16_t y,
	const int8_t velocityX,
	const int8_t velocityY,
	const int8_t accelerationX,
	const int8_t accelerationY,
	const int8_t spriteIndex,
	const int8_t timer
	) {
	
	for (uint8_t i = 0; i < PARTICLES_MAX; ++i) {
		baseParticle* baseP = &gs.particles[i];
		if (baseP->timer < 1) {

			baseParticle tmpPart;
			tmpPart.location.x = x;
			tmpPart.location.y = y;
			tmpPart.velocity[X_AXIS] = velocityX;
			tmpPart.velocity[Y_AXIS] = velocityY;
			tmpPart.acceleration[X_AXIS] = accelerationX;
			tmpPart.acceleration[Y_AXIS] = accelerationY;
			tmpPart.spriteIndex = spriteIndex;
			tmpPart.timer = timer;
			
			gs.particles[i] = tmpPart;
			return;
		}
	}
}

void ParticlStep(void) {
	for (uint8_t i = 0; i < PARTICLES_MAX; ++i) {

		baseParticle* baseP = &gs.particles[i];
		if (baseP->timer > 0) {

			--(baseP->timer);
			baseP->velocity[X_AXIS] += baseP->acceleration[X_AXIS];
			baseP->velocity[Y_AXIS] += baseP->acceleration[Y_AXIS];
			baseP->location.x += baseP->velocity[X_AXIS];
			baseP->location.y += baseP->velocity[Y_AXIS];

			//despawn off screen
			if (baseP->location.x >= TO_FIXPOINT(BASE_RES_WIDTH - SPRITE_WIDTH) 
				|| baseP->location.y >= TO_FIXPOINT(BASE_RES_HEIGHT - SPRITE_HEIGHT) 
				|| 0 == baseP->location.x 
				|| 0 == baseP->location.y) {
				baseP->timer = 0;
			}

		}//end if timer is still active
	}//end main loop
}

void DrawPartics(void) {
	for (uint8_t i = 0; i < PARTICLES_MAX; ++i) {
		baseParticle* baseP = &gs.particles[i];
		if (baseP->timer > 0) {
			DrawSprite(baseP->location.x, baseP->location.y, baseP->spriteIndex, false, i & 1, false, false, false);
		}
	}
}

#endif