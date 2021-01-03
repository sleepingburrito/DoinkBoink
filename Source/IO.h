#pragma once
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "GlobalState.h"
#include "typedefs.h"
#include "Rewind.h"

void InitJoyPads(void) {
	//trys to populate joyPads array
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		joyPads[i] = SDL_JoystickOpen(i);
	}

	ZeroOut((uint8_t*)&padIO, sizeof(padIO));
	
	windowFullscreen = false;
	pauseIoTimer = 0;
}

void CloseJoypads(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		if (SDL_JoystickGetAttached(joyPads[i])) {
			SDL_JoystickClose(joyPads[i]);
		}
	}

	ZeroOut((uint8_t*)&padIO, sizeof(padIO));
}

void UpdateKeyStates(void) {
	const Uint8* keyboard = SDL_GetKeyboardState(NULL);

	//house keeping keybord functions 
	const bool isAltDown = (keyboard[SDL_SCANCODE_LALT] || keyboard[SDL_SCANCODE_RALT]);
	if (
		(isAltDown && keyboard[SDL_SCANCODE_F4])
		|| keyboard[SDL_SCANCODE_ESCAPE]
		) {
		quitGame = true;
	}

	if (isAltDown && keyboard[SDL_SCANCODE_RETURN] && 0 == pauseIoTimer) {
		SetFullScreen(windowFullscreen = !windowFullscreen);
		pauseIoTimer = BLOCK_ALL_IO_TIME;
	}

	//updates padIO array
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {

		//key state buffer
		flags tempState = 0;

		//global key states
		bool startButton = false;
		bool debugGard = false;
		bool debugKeySwap = false;
		bool debugReinitJoy = false;
		bool debugRewind = false;
		bool debugChangeMap = false;

		//io lockout timer
		if (0 == pauseIoTimer){
			//keybord

			//moving
			if (keyboard[i ? P1_KEY_UP : P2_KEY_UP]) {
				FLAG_SET(tempState, PAD_UP);
			}
			if (keyboard[i ? P1_KEY_DOWN : P2_KEY_DOWN]) {
				FLAG_SET(tempState, PAD_DOWN);
			}
			if (keyboard[i ? P1_KEY_LEFT : P2_KEY_LEFT]) {
				FLAG_SET(tempState, PAD_LEFT);
			}
			if (keyboard[i ? P1_KEY_RIGHT : P2_KEY_RIGHT]) {
				FLAG_SET(tempState, PAD_RIGHT);
			}
			//jump
			if (keyboard[i ? P1_KEY_JMP : P2_KEY_JMP]) {
				FLAG_SET(tempState, PAD_JUMP);
			}
			//action
			if (keyboard[i ? P1_KEY_ACTION : P2_KEY_ACTION]) {
				FLAG_SET(tempState, PAD_ACTION);
			}
			//run
			if (keyboard[i ? P1_KEY_RUN : P2_KEY_RUN]) {
				FLAG_SET(tempState, PAD_RUN);
			}
			//dodge
			if (keyboard[i ? P1_KEY_DODGE : P2_KEY_DODGE]) {
				FLAG_SET(tempState, PAD_DODGE);
			}
			//start
			if (keyboard[KEY_PAUSE]) {
				startButton = true;
			}
			//debug gard
			if (keyboard[KEY_DEBUG_START]) {
				debugGard = true;
			}
			//swap
			if (keyboard[KEY_DEBUG_SWAP]) {
				debugKeySwap = true;
			}
			//reinit joypads
			if (keyboard[KEY_DEBUG_REINIT]) {
				debugReinitJoy = true;
			}
			//rewind
			if (keyboard[KEY_DEBUG_REWIND]) {
				debugReinitJoy = true;
			}
			//switch maps
			if (keyboard[KEY_DEBUG_CHANGE_MAP]) {
				debugChangeMap = true;
			}
			//end of keybord

			//update if the contollor is seen
			if (SDL_JoystickGetAttached(joyPads[i])) {

				//direction analog
				int16_t dirx = SDL_JoystickGetAxis(joyPads[i], i ? P2_PAD_AXIS_X : P1_PAD_AXIS_X)
					| SDL_JoystickGetAxis(joyPads[i], i ? P2_PAD_AXIS_X_ALT : P1_PAD_AXIS_X_ALT);
				int16_t diry = SDL_JoystickGetAxis(joyPads[i], i ? P2_PAD_AXIS_Y : P1_PAD_AXIS_Y)
					| SDL_JoystickGetAxis(joyPads[i], i ? P2_PAD_AXIS_Y_ALT : P1_PAD_AXIS_Y_ALT);

				//dpad
				const uint8_t dpad = SDL_JoystickGetHat(joyPads[i], 0);
				if (dpad == SDL_HAT_UP || dpad == SDL_HAT_LEFTUP || dpad == SDL_HAT_RIGHTUP) {
					diry = -INT16_MAX;
				}

				if (dpad == SDL_HAT_DOWN || dpad == SDL_HAT_LEFTDOWN || dpad == SDL_HAT_RIGHTDOWN) {
					diry = INT16_MAX;
				}

				if (dpad == SDL_HAT_LEFT || dpad == SDL_HAT_LEFTUP || dpad == SDL_HAT_LEFTDOWN) {
					dirx = -INT16_MAX;
				}

				if (dpad == SDL_HAT_RIGHT || dpad == SDL_HAT_RIGHTUP || dpad == SDL_HAT_RIGHTDOWN) {
					dirx = INT16_MAX;
				}

				//left
				const int16_t tmpDeadzon = i ? P2_DEADZONE : P1_DEADZONE;
				if (dirx < -tmpDeadzon) {
					FLAG_SET(tempState, PAD_LEFT);
				}
				//right
				if (dirx > tmpDeadzon) {
					FLAG_SET(tempState, PAD_RIGHT);
				}
				//up
				if (diry < -tmpDeadzon) {
					FLAG_SET(tempState, PAD_UP);
				}
				//down
				if (diry > (i ? P2_DEADZONE_DOWN : P1_DEADZONE_DOWN)) {
					FLAG_SET(tempState, PAD_DOWN);
				}

				//jump
				if (SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_JMP : P1_PAD_BUTTON_JMP)
					|| SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_JMP_ALT : P1_PAD_BUTTON_JMP_ALT)) {
					FLAG_SET(tempState, PAD_JUMP);
				}
				//action
				if (SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_ACTION : P1_PAD_BUTTON_ACTION)
					|| SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_ACTION_ALT : P1_PAD_BUTTON_ACTION_ALT)) {
					FLAG_SET(tempState, PAD_ACTION);
				}
				//run
				if (SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_RUN : P1_PAD_BUTTON_RUN)
					|| SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_RUN_ALT : P1_PAD_BUTTON_RUN_ALT)) {
					FLAG_SET(tempState, PAD_RUN);
				}
				//dodge
				if (SDL_JoystickGetButton(joyPads[i], i ? P2_PAD_BUTTON_DODGE : P1_PAD_BUTTON_DODGE)
					|| SDL_JoystickGetAxis(joyPads[i], i ? P2_PAD_BUTTON_DODGE_ALT : P1_PAD_BUTTON_DODGE_ALT) > tmpDeadzon) {
					FLAG_SET(tempState, PAD_DODGE);
				}
				//start 
				if (SDL_JoystickGetButton(joyPads[i], PAD_PAUSE)) {
					startButton = true;
				}

				//debug gard
				if (SDL_JoystickGetButton(joyPads[i], PAD_DEBUG_START)) {
					debugGard = true;

					//debug swap
					if (SDL_JoystickGetButton(joyPads[i], PAD_DEBUG_SWAP)) {
						debugKeySwap = true;
					}

					//debug reinit
					if (SDL_JoystickGetButton(joyPads[i], PAD_DEBUG_REINIT)) {
						debugReinitJoy = true;
					}

					//debug rewind
					if (SDL_JoystickGetButton(joyPads[i], PAD_DEBUG_REWIND)) {
						debugRewind = true;
					}

					//switch maps
					if (SDL_JoystickGetButton(joyPads[i], PAD_DEBUG_CHANGE_MAP)) {
						debugChangeMap = true;
					}
				}//end of debug hold

				//AI fight debug
				if (debugGard && FLAG_TEST(tempState, PAD_JUMP)) {
					if (FLAG_TEST(gs.players[i].AI, AI_ENABLED)) {
						FLAG_ZERO(gs.players[i].AI, AI_ENABLED);
					}
					else {
						FLAG_SET(gs.players[i].AI, AI_ENABLED);
					}
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}
				//AI fetch debug
				if (debugGard && FLAG_TEST(tempState, PAD_RUN)) {
					if (FLAG_TEST(gs.players[i].AI, AI_FETCH)) {
						FLAG_ZERO(gs.players[i].AI, AI_ENABLED);
						FLAG_ZERO(gs.players[i].AI, AI_FETCH);
					}
					else {
						FLAG_SET(gs.players[i].AI, AI_ENABLED);
						FLAG_SET(gs.players[i].AI, AI_FETCH);
					}
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}

			}//end of joypad loops

			//pause
			if (startButton) {
				if (screen == SCREEN_STATE_GAME_PAUSED) {
					screen = screenStateSavePause;
				}
				else {
					screenStateSavePause = screen;
					screen = SCREEN_STATE_GAME_PAUSED;
				}
				pauseIoTimer = BLOCK_ALL_IO_TIME;
			}

			//global/debug key states
			if (debugGard) {
				//debug swap
				if (debugKeySwap) {
					SDL_Joystick* const tmpJoy = joyPads[0];
					joyPads[0] = joyPads[1];
					joyPads[1] = tmpJoy;
					ZeroOut((uint8_t*)&padIO, sizeof(padIO));
					pauseIoTimer = BLOCK_ALL_IO_TIME;
					return;
				}

				//debug reinit
				if (debugReinitJoy) {
					CloseJoypads();
					InitJoyPads();
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}

				//debug rewind
				if (debugRewind) {
					if (screen != SCREEN_STATE_REWIND) {
						RewindEnter();
					}
					else {
						RewindExit();
					}
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}

				//switch maps
				if (debugChangeMap) {
					if (++newMapIndex >= MAP_COUNT) {
						newMapIndex = 0;
					}
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}

				//music
				if (FLAG_TEST(tempState, PAD_ACTION)) {
					if (muteMusic) {
						SetMuteMusic(false);
						SetPlayRandomMusic(true);
					}
					else {
						SetMuteMusic(true);
					}
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}

				//slowmo
				if (FLAG_TEST(tempState, PAD_DODGE)) {
					++updateLogicRateTimeShifter;
					updateLogicRateTimeShifter &= MS_TIME_SHIFT_MASK;
					pauseIoTimer = BLOCK_ALL_IO_TIME;
				}

			}//end of debug gard
			//end of global/debug key states

		}//end of debug key actions and getting physical key state

		//key states logic
		padIO[PADIO_INDEX(PAD_STATE_HELD, i)] = tempState;
		padIO[PADIO_INDEX(PAD_STATE_TAP, i)] = ~padIO[PADIO_INDEX(PAD_DELTA_BUFFER, i)] & tempState;
		padIO[PADIO_INDEX(PAD_DELTA_BUFFER, i)] = tempState;

	}//player 1/2 loop

	//update game IO array only when in gameplay screen
	if (screen == SCREEN_STATE_GAME) {
		ByteCopy((uint8_t*)&padIO, (uint8_t*)&gs.padIOReadOnly, IO_STATE_COUNT_MAX);
	}

	//dec io blocking timer
	if (pauseIoTimer != 0) {
		--pauseIoTimer;
	}

}
