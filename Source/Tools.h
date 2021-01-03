#pragma once
#include <SDL.h>
#include <stdint.h>
#include "Const.h"
#include "typedefs.h"

//macros
#define REMOVE_FIXPOINT(rawLoc) ((rawLoc) >> FIX_POINT_OFFSET)
#define TO_FIXPOINT(val) ((val) << FIX_POINT_OFFSET)
#define ENUM_TO_MASK_BIT(ENUM)  (1 << (ENUM))
#define ENUM_TO_MASK_BYTE(ENUM)  (8 *(ENUM))

#define FLAG_SET(flagToSet, enumVal) ((flagToSet) |= ENUM_TO_MASK_BIT(enumVal))
#define FLAG_ZERO(flagToZero, enumVal) ((flagToZero) &= ~ENUM_TO_MASK_BIT(enumVal))
#define FLAG_TEST(flagToTest, enumVal) ((((flagToTest) & ENUM_TO_MASK_BIT(enumVal)) != 0))

#define POW2(x) ((x)*(x))

//io.h
#define PADIO_INDEX(PAD_STATE, PLAYER) ((PAD_STATE) + (PLAYER) * PAD_STATE_COUNT)

//end of macros

//mem tools
void ZeroOut(uint8_t * const zeroMe, const size_t byteCount) {
	if (zeroMe == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("ZeroOut Null ptr");
		assert(false);
#endif
	}

	for (size_t i = 0; i < byteCount; ++i) {
		zeroMe[i] = 0;
	}
}

void ByteCopy(const uint8_t* const from, uint8_t* const to, const size_t byteCount) {
	if (from == NULL || to == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("ByteCopy Null ptr");
		assert(false);
#endif
	}

	for (size_t i = 0; i < byteCount; ++i) {
		to[i] = from[i];
	}
}

//timing tools
//these are in miliseconds
int64_t MsClock(void) {
	//how many ms the app has been running
	static int64_t MasterMsClock = 0;
	static int32_t timeLast = 0;
	static int64_t MasterMsClockOld = 0;
	int32_t delta = 0;

	if ((int32_t)SDL_GetTicks() >= timeLast) {
		delta = SDL_GetTicks() - timeLast;
	}
	else {
		delta = UINT32_MAX - timeLast + SDL_GetTicks();
	}

	timeLast += delta;
	MasterMsClock += delta;

	if (MasterMsClockOld > MasterMsClock) {
		printf("MasterMsClock overflow\n");
		assert(false);
	}
	return MasterMsClockOld = MasterMsClock;
}

//void DelayMainLoop(void) {
//	//limit update rate of loop and tick ms clock
//	//note only run once a frame
//	static int64_t updateRateTimer = 0;
//	
//	static int64_t frameTimeTimer = 0;
//	static int32_t frameTime = 0;
//	
//	if (MsClock() < updateRateTimer) {
//		//limit to MS_TILL_UPDATE
//		const int32_t delay = (int32_t)(updateRateTimer - MsClock());
//		SDL_Delay(delay);
//	}
//	updateRateTimer = MsClock() + (MS_TILL_UPDATE);
//}

int32_t FPScounterMs(void) {
	//run only once evey frame, after 1 second it will have accurite fps
	static int64_t fpsCounterTimer = 0;
	static int32_t fps = 0;
	static int32_t fpsDisp = 0;
	++fps;
	if (fpsCounterTimer < MsClock()) {
		fpsDisp = fps;
		fpsCounterTimer = MsClock() + 1000;
		fps = 0;
	}
	return fpsDisp;
}

//double PrefFpsCounter(void) {
//	static Uint64 lastframe = 1;
//	static Uint64 delta = 1;
//	
//	delta = (SDL_GetPerformanceCounter() - lastframe) ;
//	lastframe = SDL_GetPerformanceCounter();
//	
//	return 1.0 / ((double)delta / (double)SDL_GetPerformanceFrequency());
//}
//
//double PrefDeltaTime(void) {
//	static Uint64 lastframe = 1;
//	static Uint64 delta = 1;
//
//	delta = (SDL_GetPerformanceCounter() - lastframe);
//	lastframe = SDL_GetPerformanceCounter();
//
//	return	((double)delta / (double)SDL_GetPerformanceFrequency());
//}
//
//Uint64 PrefDeltaTimeRaw(void) {
//	static Uint64 lastframe = 0;
//	Uint64 delta = 0;
//
//	delta = (SDL_GetPerformanceCounter() - lastframe);
//	lastframe = SDL_GetPerformanceCounter();
//
//	return	delta;
//}
//
//Uint64 Pref60Hz(void) {
//	return SDL_GetPerformanceFrequency() / 61; //61 for overhead
//}

//end of timing tools

//flag tools
flags BitCopy(const flags flagSource, const uint8_t offsetSource, const flags flagDest, const uint8_t offsetDest) {
	return (flagDest & ~(1 << offsetDest)) | (((flagSource >> offsetSource) & 1) << offsetDest);
}

//math
void AddUint8Capped(uint8_t * const current, const uint8_t add) {
	if (current == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("AddUint8Capped Null ptr");
		assert(false);
#endif
	}

	const uint8_t old = *current;
	*current += add;
	if (*current < old) {
		*current = UINT8_MAX;
	}
}

void AddInt8Capped(int8_t* const current, const int8_t add) {
	if (current == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("AddInt8Capped Null ptr");
		assert(false);
#endif
	}
	//copy sign
	const int8_t sign = 128 & *current;
	//add
	*current += add;
	//check for overflow
	if ((int8_t)(128 & *current) != sign) {
		*current = UINT8_MAX & sign;
	}
}

//find Distance leaving off the sq root
int32_t DistancePart(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	const int32_t x = (int32_t)x1 - (int32_t)x2;
	const int32_t y = (int32_t)y1 - (int32_t)y2;
	return x * x + y * y;
}
//end of math

//frame timing tools
void DiscernmentAllTimers(timer* timers, uint8_t count) {
	if (timers == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("DiscernmentAllTimers Null ptr");
		assert(false);
#endif
	}

	//remove 1 from all timers but dont go below zero
	for (uint8_t i = 0; i < count; ++i) {
		if (timers[i] > 0) {
			timers[i] -= 1;
		}
	}
}
//end of frame timeing tools

//box tools
//note x/y are in fixpoint
void CheckBoxErrors(const boxWorldSpace* const boxIn) {

	if (boxIn == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("CheckBoxErrors Null ptr");
		assert(false);
#endif
	}

	if (boxIn->topLeft.x >= TO_FIXPOINT(BASE_RES_WIDTH)
		|| boxIn->bottomRight.x >= TO_FIXPOINT(BASE_RES_WIDTH)
		|| boxIn->topLeft.y >= TO_FIXPOINT(BASE_RES_HEIGHT)
		|| boxIn->bottomRight.y >= TO_FIXPOINT(BASE_RES_HEIGHT)) {
#ifdef NDEBUG
		return;
#else
		printf("box out of map");
		assert(false);
#endif
	}

	if (boxIn->topLeft.x + boxIn->boxSize.width != boxIn->bottomRight.x
		|| boxIn->topLeft.y + boxIn->boxSize.height != boxIn->bottomRight.y) {
#ifdef NDEBUG
		return;
#else
		printf("box sides are missalinged");
		assert(false);
#endif
	}

	if (boxIn->boxSize.width <= REMOVE_FIXPOINT(MAX_SPEED)
		|| boxIn->boxSize.height <= REMOVE_FIXPOINT(MAX_SPEED)) {
#ifdef NDEBUG
		return;
#else
		printf("box too small");
		assert(false);
#endif
	}
}

boxWorldSpace InitBox(const location x, const location y, const dimension height, const dimension width) {
	boxWorldSpace returnBox;
	returnBox.topLeft.x = x;
	returnBox.topLeft.y = y;
	returnBox.boxSize.height = height;
	returnBox.boxSize.width = width;
	returnBox.bottomRight.y = returnBox.topLeft.y + returnBox.boxSize.height;
	returnBox.bottomRight.x = returnBox.topLeft.x + returnBox.boxSize.width;
	CheckBoxErrors(&returnBox);
	return returnBox;
}

void SetBox(boxWorldSpace * const boxIn, const location x, const location y) {
	boxIn->topLeft.x = x;
	boxIn->topLeft.y = y;
	boxIn->bottomRight.x = x + boxIn->boxSize.width;
	boxIn->bottomRight.y = y + boxIn->boxSize.height;
#ifdef NDEBUG
	return;
#else
	CheckBoxErrors(boxIn);
#endif
}

void MoveBoxRelative(boxWorldSpace * const boxIn, const int16_t x, const int16_t y) {
	SetBox(boxIn, boxIn->topLeft.x + x, boxIn->topLeft.y + y);
}

bool BoxOverlap(const boxWorldSpace * const boxInA, const boxWorldSpace * const boxInB) {
#ifdef NDEBUG
#else
	CheckBoxErrors(boxInA);
	CheckBoxErrors(boxInB);
#endif

	return !(boxInA->topLeft.x   >= boxInB->bottomRight.x ||
		     boxInA->bottomRight.x <= boxInB->topLeft.x   ||
		     boxInA->topLeft.y   >= boxInB->bottomRight.y ||
		     boxInA->bottomRight.y <= boxInB->topLeft.y);
}

void RightOfBox(boxWorldSpace * const moveMe, const boxWorldSpace * const toHere){
#ifdef NDEBUG
#else
	CheckBoxErrors(moveMe);
	CheckBoxErrors(toHere);
#endif
	SetBox(moveMe, toHere->bottomRight.x, moveMe->topLeft.y);
}

void LeftOfBox(boxWorldSpace * const moveMe, const boxWorldSpace * const toHere) {
#ifdef NDEBUG
#else
	CheckBoxErrors(moveMe);
	CheckBoxErrors(toHere);
#endif
	SetBox(moveMe, toHere->topLeft.x - moveMe->boxSize.width, moveMe->topLeft.y);
}

void BottomOfBox(boxWorldSpace * const moveMe, const boxWorldSpace * const toHere) {
#ifdef NDEBUG
#else
	CheckBoxErrors(moveMe);
	CheckBoxErrors(toHere);
#endif
	SetBox(moveMe, moveMe->topLeft.x, toHere->bottomRight.y);
}

void TopOfBox(boxWorldSpace * const moveMe, const boxWorldSpace * const toHere) {
#ifdef NDEBUG
#else
	CheckBoxErrors(moveMe);
	CheckBoxErrors(toHere);
#endif
	SetBox(moveMe, moveMe->topLeft.x, toHere->topLeft.y - moveMe->boxSize.height);
}

void BoxMoveHalfWay(boxWorldSpace* const boxMove, const boxWorldSpace* const boxMoveTo) {
	//Sets boxMove to be halways between its current locaion and boxMoveTo
	//used for linear interpolation (like to smooth out slow mo smoothing)
	//note: use for display only. needs more testing, has bugs

	//zero in a move two is buggy, skip it if you find one
	if (boxMove->topLeft.x == 0 || boxMove->topLeft.y == 0) {
		return;
	}

	if (boxMoveTo->topLeft.x == 0 || boxMoveTo->topLeft.y == 0) {
		return;
	}

	const uint16_t x = (int16_t)boxMove->topLeft.x + (((int16_t)boxMove->topLeft.x - (int16_t)boxMoveTo->topLeft.x) >> 1);
	const uint16_t y = (int16_t)boxMove->topLeft.y + (((int16_t)boxMove->topLeft.y - (int16_t)boxMoveTo->topLeft.y) >> 1);

	SetBox(boxMove, x, y);
}
//end of box tools

