#pragma once
#include "typedefs.h"
#include "GlobalState.h"
#include "BallCode.h"
#include "PlayerCode.h"

//for now rewind is mostly a debug tool

void InitRewind(void) {
	ZeroOut((uint8_t*)tap, sizeof(tap));
	tapFrame = 0;
}

void RewindStep(void) {
	const flags keys = padIO[PADIO_INDEX(PAD_STATE_TAP, PLAYER_ONE)] | padIO[PADIO_INDEX(PAD_STATE_TAP, PLAYER_TWO)];

	if (FLAG_TEST(keys, PAD_LEFT)) {
		--tapFrame;
	}
	if (FLAG_TEST(keys, PAD_RIGHT)) {
		++tapFrame;
	}
	if (FLAG_TEST(keys, PAD_UP)) {
		tapFrame += 10;
	}
	if (FLAG_TEST(keys, PAD_DOWN)) {
		tapFrame -= 10;
	}
	if (FLAG_TEST(keys, PAD_ACTION)) { //play from this frame
		screen = SCREEN_STATE_GAME;
	}

	gs = tap[tapFrame];
}

void RewindRecord(void) {
	tap[tapFrame++] = gs;
}

void RewindEnter(void) {
	screen = SCREEN_STATE_REWIND;
	tapFrameLast = --tapFrame;
}

void RewindExit(void) {
	screen = SCREEN_STATE_GAME;
	gs = tap[tapFrame = tapFrameLast];
}


//instant replay, used at the end of the match
//start a replay by setting the timer
//ex: replayStartTimer = REPLAY_START_IN;

//used outside of the replay screen state to get to it
void ReplayStartStep(void){
	if (replayStartTimer > 0) {
		if (--replayStartTimer == 1) {
			replayStartTimer = 0;
			screen = SCREEN_STATE_INSTANT_REPLAY;
			tapFrameLast = tapFrame;
			tapFrame -= REPLAY_FRAME_START;
		}
	}
}

//does the replay and restarts the game tat the end
void ReplayScreenStep(void) {

	mainState dispStateBuffer = tap[tapFrame];
	const uint8_t nextFrameIndex = tapFrame + (uint8_t)1;

	//halve the playback speed/smooth frames by making up new ones
	//note: particles and anything not listed below still run at half framerate
	if (
		(++replaySlowMo & 1) 
		&& (nextFrameIndex != tapFrameLast) //dont play first frame of a loop
	){
		const mainState dispStateBufferNext = tap[nextFrameIndex];
		//make up frame data for half a frame to smooth playback
		//players
		for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
			BoxMoveHalfWay(&dispStateBuffer.players[i].playerPhysics.postionWorldSpace, &dispStateBufferNext.players[i].playerPhysics.postionWorldSpace);
		}
		//ball
		BoxMoveHalfWay(&dispStateBuffer.ball.ballPhysics.postionWorldSpace, &dispStateBufferNext.ball.ballPhysics.postionWorldSpace);
	}
	else { //inc to next frame
		++tapFrame;
	}

	//update gs to dosplay new frame
	gs = dispStateBuffer;

	//restart the game at replay end
	if (tapFrame == tapFrameLast) {
		screen = SCREEN_STATE_GAME;
		InitPlayers();
		InitBall();
		InitParticles();
	}
}