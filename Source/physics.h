#pragma once
#include "Tools.h"
#include <stdint.h>
#include <stdbool.h>
#include "GlobalState.h"
#include "Const.h"
#include "typedefs.h"

//speed tools
void ZeroOutSpeed(speed* const in) {
	if (in == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("ZeroOutSpeed Null ptr");
		assert(false);
#endif
	}

	for (uint8_t i = 0; i < SPEED_INDEX_COUNT; ++i) {
		in[i] = 0;
	}
}

void CopySpeed(const speed* const source, speed* const dest) {
	if (source == NULL || dest == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("CopySpeed Null ptr");
		assert(false);
#endif
	}

	for (uint8_t i = 0; i < SPEED_INDEX_COUNT; ++i) {
		dest[i] = source[i];
	}
}

//to give 'recoil'
void SpeedAddInvert(const speed* const source, speed* const dest, const uint8_t shiftAmount) {
	if (source == NULL || dest == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("SpeedAddInvert Null ptr");
		assert(false);
#endif
	}

	AddUint8Capped(&dest[SPEED_UP_INDEX], source[SPEED_DOWN_INDEX] >> shiftAmount);
	AddUint8Capped(&dest[SPEED_DOWN_INDEX], source[SPEED_UP_INDEX] >> shiftAmount);
	AddUint8Capped(&dest[SPEED_LEFT_INDEX], source[SPEED_RIGHT_INDEX] >> shiftAmount);
	AddUint8Capped(&dest[SPEED_RIGHT_INDEX], source[SPEED_LEFT_INDEX] >> shiftAmount);
}

void CopySpeedHalf(const speed* const source, speed* const dest) {
	if (source == NULL || dest == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("CopySpeedHalf Null ptr");
		assert(false);
#endif
	}

	for (uint8_t i = 0; i < SPEED_INDEX_COUNT; ++i) {
		dest[i] = source[i] >> 1;
	}
}

void AddSpeeds(const speed* const source, speed* const dest) {
	if (source == NULL || dest == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("AddSpeeds Null ptr");
		assert(false);
#endif
	}

	for (uint8_t i = 0; i < SPEED_INDEX_COUNT; ++i) {
		AddUint8Capped(&dest[i], source[i]);
	}
}

void ApplyFriction(speed* const dest, const uint8_t* const friction) {
	if (dest == NULL || friction == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("ApplyFriction Null ptr");
		assert(false);
#endif
	}
	
	for (uint8_t i = 0; i < SPEED_INDEX_COUNT; ++i) {
		dest[i] = friction[dest[i]];
	}
}


//makes a number that till let you compair two speeds
int32_t SpeedCompare(const speed * const speedsin) {
	if (speedsin == NULL) {
#ifdef NDEBUG
		return 0;
#else
		printf("SpeedCompare Null ptr");
		assert(false);
#endif
	}

	const int32_t hspeed = -(int16_t)speedsin[SPEED_LEFT_INDEX] + (int16_t)speedsin[SPEED_RIGHT_INDEX];
	const int32_t vspeed = -(int16_t)speedsin[SPEED_UP_INDEX] + (int16_t)speedsin[SPEED_DOWN_INDEX];
	return (hspeed * hspeed) + (vspeed * vspeed);
}

//speed up ideas, skip axis if no speed
//exit axis ealy if it only 1 wall
void PhysicsStep(physicsState* const in) {
	if (in == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("PhysicsStep Null ptr");
		assert(false);
#endif
	}

	//flag reset
	FLAG_ZERO(in->physicsFlags, PHYSICS_ONGROUND);
	FLAG_ZERO(in->physicsFlags, PHYSICS_IN_WALL);
	FLAG_ZERO(in->physicsFlags, PHYSICS_IN_HORIZONTAL_WALL);

	//moving
	for (uint8_t axis = 0; axis < AXIS_COUNT; ++axis) {
		speed* postiveSpeed = NULL;
		speed* negativeSpeed = NULL;
		//set speed of axis
		if (axis == X_AXIS) {
			postiveSpeed = &in->allSpeeds[SPEED_RIGHT_INDEX];
			negativeSpeed = &in->allSpeeds[SPEED_LEFT_INDEX];
		}
		else { //y axis
			postiveSpeed = &in->allSpeeds[SPEED_DOWN_INDEX];
			negativeSpeed = &in->allSpeeds[SPEED_UP_INDEX];
		}

		//find speed
		const int16_t tmpSpeed = -(int16_t)(*negativeSpeed) + (*postiveSpeed);

		//if moving
		if (tmpSpeed != 0) {
			//move object
			if (axis == X_AXIS) {
				MoveBoxRelative(&in->postionWorldSpace, tmpSpeed, 0);
			}
			else { //y axis
				MoveBoxRelative(&in->postionWorldSpace, 0, tmpSpeed);
			}
			//get signing
			const bool sign = tmpSpeed < 0;
			//test for walls
			for (uint8_t i = 0; i < gs.mapBoxCount; ++i) {
				//if you hit a wall
				if (BoxOverlap(&in->postionWorldSpace, &gs.map[i])) {
					//mark in wall
					FLAG_SET(in->physicsFlags, PHYSICS_IN_WALL);
					//bouncing
					if (FLAG_TEST(in->physicsFlags, PHYSICS_BOUNCE))
					{
						speed tmp = AIR_FRICTION[*negativeSpeed];
						*negativeSpeed = AIR_FRICTION[*postiveSpeed];
						*postiveSpeed = tmp;

					}
					else {
						//stop obj from moving
						*negativeSpeed = 0;
						*postiveSpeed = 0;
					}
					if (axis == X_AXIS) {
						
						FLAG_SET(in->physicsFlags, PHYSICS_IN_HORIZONTAL_WALL);
						
						if (sign) { //left
							RightOfBox(&in->postionWorldSpace, &gs.map[i]);
						}
						else { //right
							LeftOfBox(&in->postionWorldSpace, &gs.map[i]);
						}
					}
					else { //y axis
						if (sign) { //going up
							BottomOfBox(&in->postionWorldSpace, &gs.map[i]);
						}
						else { //going down
							TopOfBox(&in->postionWorldSpace, &gs.map[i]);
							FLAG_SET(in->physicsFlags, PHYSICS_ONGROUND); //set on ground
						}
					}//end of y axis
					//should be able to hit only 1 wall per axis at a time
					break;
				}//end if hit wall
			}//end of wall checking
		}//end of if moving

	}//end of axis loop

	 //gravity
	if (!FLAG_TEST(in->physicsFlags, PHYSICS_DISABLE_GRAVITY)) {
		AddUint8Capped(&in->allSpeeds[SPEED_DOWN_INDEX], GRAVITY);
	}

	//friction
	const uint8_t* tmpFrictionTable = GROUND_FRICTION; //default to GROUND_FRICTION
	if (!FLAG_TEST(in->physicsFlags, PHYSICS_IN_WALL)) {
		tmpFrictionTable = AIR_FRICTION;
	}
	if (FLAG_TEST(in->physicsFlags, PHYSICS_BRAKES)) {
		tmpFrictionTable = BRAKES_FRICTION;
	}
	ApplyFriction(in->allSpeeds, tmpFrictionTable);

}