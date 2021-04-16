//C libs
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

//SDL and SDL Addons
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

//Game Files
#include "Const.h"
#include "GameStructs.h"
#include "GlobalState.h"
#include "RandomNG.h"
#include "Tools.h"
#include "Sound.h"
#include "Graphics.h"
#include "Map.h"
#include "Particles.h"
#include "Rewind.h"
#include "GamePad.h"
#include "Physics.h"
#include "BaseCollision.h"
#include "Ball.h"
#include "Player.h"


void InitGameOnly(void) {
	//Reset game and keep SDL running

	InitMap(); //Call InitMap() and LoadMap() before calling the players or ball init (map holds spawn locations)
	LoadMap(gs.mapIndex);
	InitPlayers();
	InitBall();
	InitBaseCollision();
	InitParticles();
	InitGameClock();
	screen = SCREEN_STATE_GAME;
}

//--Main game loop functions--
//
void UpdateWatchdog(void) {
	//sends a signal that the game is not frozen
	++WatchDogFrameCount;
	if (0 == (WatchDogFrameCount & WATCHDOD_MASK)) {
		printf(WATCHDOG_MSG" %d \n", WatchDogFrameCount); 
	}
}

void PullSDLevents(void) {
	//update SDL events, needs to happen to keep SDL happy
	SDL_Event event;
	SDL_PollEvent(&event);
	if (SDL_QUIT == event.type) {
		//if the user presses the X on the window quit
		quitGame = true;
	}
}

void LimitGameSpeed(void) {
	bool fallbackGameLogicLimit = false;
	const uint64_t timeDeltaLastFrame = MsClock() - lastLogicUpdate;

	//check if the game is running too fast, if so enable logic limit
	if (timeDeltaLastFrame < (uint64_t)(MS_TILL_UPDATE)) {
		fallbackGameLogicLimit = true;
	}

	//check timer for game logic speed limiter
	if (timeDeltaLastFrame >= ((uint64_t)MS_TILL_UPDATE << updateLogicRateTimeShifter)) {
		lastLogicUpdate = MsClock();
		updateGameLogic = true;
	}

	//if the display refresh rate is the same as target frame rate then remove frame limiter and use vsynce
	if (0 == updateLogicRateTimeShifter
		&& TARGET_FRAME_RATE == displayRefreshRate
		&& false == fallbackGameLogicLimit) {
		updateGameLogic = true;
	}

	if (FRAME_SKIP != 0 && (frameSkip++) >= FRAME_SKIP) {
		updateGameLogic = false;
		frameSkip = 0;
	}
}

//start screen
void ShowStartScreen(void) {
	showStartScreen = true;
}

void StopStartScreen(void) { //based off key press
	const uint8_t keys = padIO[PLAYER_ONE] | padIO[PLAYER_TWO];
	if (FLAG_TEST(keys, PAD_ACTION)) {
		gs.settingsAi[PLAYER_TWO] = gs.settingsAi[PLAYER_ONE] = AI_SET_OFF;
		InitGameOnly();
		showStartScreen = false;
		SetMuteSoundEffects(false);
	}
}

void StartScreenStep(void) {
	if (!showStartScreen) return;

	gs.settingsAi[PLAYER_TWO] = gs.settingsAi[PLAYER_ONE] = AI_SET_MEDIUM;
	SetAutoMapSwitch(true);
	StopStartScreen();

}

void DrawStartScreen(void) {
	if (!showStartScreen) return;

	const int8_t yoff = 0;//(int8_t)((float)SIN_TABLE[gs.spriteTimer] * GAME_TEXT_BOUNCE_AMT);

	DrawText(GAME_NAME_X, GAME_NAME_Y + yoff, true, GAME_NAME_STARTSCREEN);
}

//
//--End main game loop functions--


//int main(void) {
int main(int inputCount, char** inputStrings){
//int main(int argc, char* argv[]) {

	//hide warnings
	inputCount = 0;
	inputStrings = NULL;

	//one time only init
	InitWindow();
	InitJoyPads();
	InitSound();
	InitGameOnly();
	InitRewind();
	RngInit(0);

	ShowStartScreen();
	SetAutoMapSwitch(false);
	SetFullScreen(false);

	SetMuteSoundEffects(true);
	SetMuteMusic(true);
	PlayMusic(0);

	//debug turn AI on at start
	//gs.settingsAi[PLAYER_ONE] = AI_SET_EASY;
	//gs.settingsAi[PLAYER_TWO] = AI_SET_EASY;
	

	//test debug
	//printf("%d\n", sizeof(globalData));
	//InitAllPlayerData(); //weckangle


	//main loop
	do{
		//--loop upkeep--
		updateGameLogic = false;
		//UpdateWatchdog();
		PullSDLevents();
		LimitGameSpeed();

		//pause game when minimized
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(MINIMIZED_PAUSED_TIME);
			continue;
		}
		//--end of main loop upkeep--


		//game logic (is ran at 60hz)
		if (updateGameLogic) {
		//if (true){
			
			//--game logic upkeep--
			PrepRendering();
			UpdateKeyStates();
			SoundStep();
			//--end game logic upkeep--

			//--screen state--
			switch (screen) {

			case SCREEN_STATE_GAME:

				//map switch, (Note: use SwitchMap() to switch maps)
				if (newMapIndex != gs.mapIndex) {
					gs.mapIndex = newMapIndex;
					InitGameOnly();
				}

				//step
				StartScreenStep();

				if (0 == gs.worldTimers[WOULD_PAUSE_TIMER]) {
					BallStep();
					UpdateBaseCollision(); //needs to come after the ball step to fix timing issues
					PlayerSteps();
					CheckPlayersScores();
					ParticlStep();

					RewindRecord();
					ReplayStartStep();
				}

				//step world timers
				DiscernmentAllTimers(gs.worldTimers, WOLD_TIMER_COUNT);
				TickGameClock();

				//draw
				SpriteTimerTick();
				DrawBackground(gs.mapIndex);
				DrawStartScreen();

				DrawPlayers();
				DrawBall();
				DrawPartics();
				
				if (!showStartScreen) {
					DrawScore();
					DrawGameClock();
					DrawEndGameWinningText();
				}

				FadeInSolid(false);

				break; //end SCREEN_STATE_GAME


			case SCREEN_STATE_REWIND: //this is mostly for debug, it lets you do a step by step playback of gameplay
				
				//step
				RewindStep();
				
				//draw
				ClearScreenSoildColor();
				DrawMapDebug();
				
				DrawScore();
				DrawGameClock();

				DrawPlayers();
				DrawPlayersDebug();
				
				DrawBall();
				DrawBallDebug();
				
				DrawPartics();
				
				break; //end SCREEN_STATE_REWIND


			case SCREEN_STATE_INSTANT_REPLAY:

				//step
				ReplayScreenStep();

				//draw
				DrawBackground(gs.mapIndex);
				
				DrawScore();
				DrawGameClock();

				DrawPlayers();
				DrawBall();
				
				DrawPartics();
				DrawEndGameWinningText();

				break; //end SCREEN_STATE_INSTANT_REPLAY


			case SCREEN_STATE_GAME_PAUSED:
				
				//draw
				SpriteTimerTick();
				DrawBackground(gs.mapIndex);
				DrawPlayers();
				DrawText(PAUSE_DISP_X, PAUSE_DISP_Y, false, PAUSE_TEXT);

				break;


			default:
				#ifdef NDEBUG
				break;
				#else
				printf("bad screen state\n");
				assert(false);
				#endif
			}
			//--end of screen state switch case--
		

			//--last draw items--
			//if (drawFpsCounter) {
			//	DrawTextNumberAppend(FPSDISP_X, FPSDISP_Y, false, FPS_TEXT" ", FPScounterMs());
			//}
			//draw text log
			LogTextScreenTickTimers();
			LogTextScreenDraw();
			//--end last draw items--

			//test debug wreck angle
			//UpdatePad();
			//PlayerDataStep(0);
			//PlayerDataStep(1);

		}//--end of game logic--

		//renders all the draw calls to screen
		DrawRenderToScreen();

	} while (!quitGame);
	//end of main game loop

	//exit / denit
	CloseJoypads();
	CloseSound();
	ShutdownWindow();
	return EXIT_SUCCESS;
}