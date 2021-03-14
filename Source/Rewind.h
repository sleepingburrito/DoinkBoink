#ifndef REWIND_H
#define REWIND_H

#include "Ball.h"
#include "Player.h"

//for now rewind is mostly a debug tool

void InitRewind(void) {
	ZeroOut((uint8_t*)tape, sizeof(tape));
	tapeFrame		 = 0;
	tapeFrameLast	 = 0;
	recordInitTime	 = 0;
	replayStartTimer = 0;
	replaySlowMo	 = 0;
}

void RewindStep(void) {

	//if buffer not full then don't execute rewind
	if (recordInitTime != UINT8_MAX - 1) {
		screen = SCREEN_STATE_GAME;
		return;
	}

	const flags keys = padIO[PADIO_INDEX(PAD_STATE_TAP, PLAYER_ONE)] | padIO[PADIO_INDEX(PAD_STATE_TAP, PLAYER_TWO)];

	if (FLAG_TEST(keys, PAD_LEFT)) {
		--tapeFrame;
	}
	if (FLAG_TEST(keys, PAD_RIGHT)) {
		++tapeFrame;
	}
	if (FLAG_TEST(keys, PAD_UP)) {
		tapeFrame += REWIND_FRAM_SKIP_FAST;
	}
	if (FLAG_TEST(keys, PAD_DOWN)) {
		tapeFrame -= REWIND_FRAM_SKIP_FAST;
	}
	if (FLAG_TEST(keys, PAD_ACTION)) { //play from this frame
		screen = SCREEN_STATE_GAME;
	}

	gs = tape[tapeFrame];
}

void RewindRecord(void) {
	tapeFrame++;
	tape[tapeFrame] = gs;
	
	//let the game know it filled the record buffer
	if (recordInitTime != UINT8_MAX - 1) {
		++recordInitTime;
	}
}

void RewindEnter(void) {
	screen = SCREEN_STATE_REWIND;
	tapeFrameLast = --tapeFrame;
}

void RewindExit(void) {
	screen = SCREEN_STATE_GAME;
	gs = tape[tapeFrame = tapeFrameLast];
}


//instant replay, used at the end of the match
//when in game screen state start a replay with below
//replayStartTimer = REPLAY_START_IN;

void ReplayStartStep(void){
	//used outside of the replay screen state to get to it (checks the timer)
	
	//start replay
	if (replayStartTimer > 0) {

		//if record buffer is not full dont do replay
		if (recordInitTime != UINT8_MAX - 1) {
			screen = SCREEN_STATE_GAME;
			RestartMatch();
			return;
		}

		//count down timer then start rewind
		if (--replayStartTimer == 1) {
			replayStartTimer = 0;
			screen = SCREEN_STATE_INSTANT_REPLAY;
			tapeFrameLast = --tapeFrame;
			tapeFrame -= REPLAY_FRAME_START;
		}
	}
}

void ReplayScreenStep(void) {
	//does the replay and restarts the game at the end

	//getting key states so you can skip the replay
	const uint8_t keys = padIO[PLAYER_ONE] | padIO[PLAYER_TWO];

	//getting buffer
	mainState dispStateBuffer = tape[tapeFrame];
	const uint8_t nextFrameIndex = tapeFrame + (uint8_t)1;

	//halve the playback speed and smooth frames by interpolation
	//note: particles and anything not listed below still run at half framerate
	if (
		(++replaySlowMo & 1) //make up a new frame every other frame
		//&& (nextFrameIndex != tapeFrameLast) //dont play first frame of a loop
	){
		const mainState dispStateBufferNext = tape[nextFrameIndex];
		//make up frame data for half a frame
		//players
		for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
			BoxMoveHalfWay(&dispStateBuffer.players[i].playerPhysics.postionWorldSpace, &dispStateBufferNext.players[i].playerPhysics.postionWorldSpace);
		}
		//ball
		BoxMoveHalfWay(&dispStateBuffer.ball.ballPhysics.postionWorldSpace, &dispStateBufferNext.ball.ballPhysics.postionWorldSpace);
	}
	else { //increment to next frame after the interpolated one
		++tapeFrame;
	}

	//update gs to display a new frame
	gs = dispStateBuffer;

	//restart the game at replay end
	if (tapeFrame == tapeFrameLast || FLAG_TEST(keys, PAD_ACTION)) {
		screen = SCREEN_STATE_GAME;

		//auto map switch
		if (autoMapSwitch) {
			if (++newMapIndex >= MAP_COUNT) {
				newMapIndex = 0;
			}
		}

		RestartMatch();
	}
}

void SetAutoMapSwitch(const bool set) {
	autoMapSwitch = set;
}

#endif