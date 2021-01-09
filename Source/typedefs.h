#pragma once
#include <stdint.h>
#include "Const.h"
#include <stdbool.h>

typedef uint8_t speed;
typedef uint8_t timer;
typedef uint8_t flags;
typedef uint8_t scoring;
typedef uint8_t gameScreenState;

typedef uint16_t dimension;
typedef uint16_t location;

typedef uint32_t mapRow;

typedef struct {
	location x, y;
}vec2;

typedef struct {
	dimension height, width;
}phySize;

//note all in fixed point
typedef struct {
	vec2 topLeft;
	vec2 bottomRight;
	phySize boxSize;
}boxWorldSpace;

typedef struct {
	flags physicsFlags;
	boxWorldSpace postionWorldSpace;
	speed allSpeeds[SPEED_INDEX_COUNT];
}physicsState;

typedef struct {
	flags playerFlags;
	
	scoring deathCount;
	uint16_t ballHeldCounter; //used for keeping score for how long you held the ball

	timer playerTimer[PLAYER_TIMER_COUNT];
	physicsState playerPhysics;
	boxWorldSpace playerHitBox;

	flags AI;
	vec2 GoalAIMove;
}playerBase;

typedef struct {
	flags ballFlags;
	speed thrust[SPEED_INDEX_COUNT];
	timer ballTimers[BALL_TIMER_COUNT];
	physicsState ballPhysics;
}ballBase;

typedef struct {
	vec2 location;
	int8_t velocity[AXIS_COUNT];
	int8_t acceleration[AXIS_COUNT];
	int8_t spriteIndex;
	int8_t timer;
	bool player2colors;
}baseParticle;

typedef struct {
	uint8_t timer;
	char text[TEXT_LOG_CHARS + 1]; //extra one to null term
	int32_t number;
}textLogBase;