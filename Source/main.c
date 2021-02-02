#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
//#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "Const.h"
#include "typedefs.h"
#include "GlobalState.h"
#include "RandomNG.h"
#include "Tools.h"
#include "Sound.h"
#include "Graphics.h"
#include "Map.h"
#include "Particles.h"
#include "Rewind.h"
#include "IO.h"
#include "physics.h"
#include "BaseCollision.h"
#include "BallCode.h"
#include "PlayerCode.h"


void InitGameOnly(void) {
	//use to reset the game but keep sdl running
	InitMap(); //Call InitMap() and LoadMap() before calling the players or ball init (map holds spawn locations)
	LoadMap(gs.mapIndex);
	InitPlayers();
	InitBall();
	InitBaseCollision();
	InitParticles();
	InitGameClock();
	screen = SCREEN_STATE_GAME;
}

//int main(int argc, char* argv[]) {
int main(void) {

	//printf("%s\n", argv[0]);

	//debug for physics, helps make tables
	//for (uint16_t i = 0; i < 255; ++i) {
	//	double part = 2.0 * 3.1415926 / 255.0;
	//	printf("%d,", (int8_t)(128.0 * sin(part * i)));
	//}

	//unused, here to make VS happy
	//if (argc == 0) argv = NULL;

	//start init one time only
	InitWindow();
	InitJoyPads();
	LoadSprites();
	InitSound();
	LoadSounds();
	SetMuteMusic(true);
	SetMuteSoundEffects(false);
	InitGameOnly();
	InitRewind(); //debug
	newMapIndex = MAP_DEBUG; //debug start map
	RngInit(0);
	SetAutoMapSwitch(false);
	InitPointLight();


	//tese debug turn AI on at start
	//gs.settingsAi[1] = AI_SET_EASY;
	//gs.settingsAi[0] = AI_SET_EASY;


	//debug notes to undo
	//------------------
	//undo test
	//disable AI
	//enable vsynce
	//reset default map
	//enable frame lmiter
	//disable map auto swithc



	//main loop
	do{
		//loop upkeep
		updateGameLogic = false;

		//watchdog
		++WatchDogFrameCount;
		if (0 == (WatchDogFrameCount & WATCHDOD_MASK)) {
			//printf(WATCHDOG_MSG" %d \n", WatchDogFrameCount); //debug test
		}

		//update sld events
		SDL_PollEvent(&event);
		if (SDL_QUIT == event.type) {
			//if the user presses the X on the window quit
			quitGame = true;
		}

		//paused when minimized
		while (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_PollEvent(&event);
			SDL_Delay(10);
		}

		//limit game logic speed
		bool fallbackGameLogicLimit = false;
		const uint64_t timeDeltaLastFrame = MsClock() - lastLogicUpdate;
		//check if the game is running too fast, if so enable logic limit
		if (timeDeltaLastFrame < (uint64_t)(MS_TILL_UPDATE)) {
			fallbackGameLogicLimit = true;
		}
		//game logic limiter
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
		//updateGameLogic = true; //debug no frame cap
		
		
		//end of loop upkeep


		//test debug


		//game logic
		if (updateGameLogic) {
			
			//game logic upkeep
			//set up render
			PrepRendering();
			//update io
			UpdateKeyStates();
			//sound upkeep
			SoundStep();

			//screen state
			switch (screen) {
			case SCREEN_STATE_MAIN_MENU:
				break;

			case SCREEN_STATE_GAME:

				//map switch, use SwitchMap() to switch maps
				if (newMapIndex != gs.mapIndex) {
					gs.mapIndex = newMapIndex;
					InitGameOnly();
				}

				//step
				if (gs.worldTimers[WOULD_PAUSE_TIMER] == 0) {
					
					BallsStep();
					UpdateBaseCollision(); //needs to come after the ball step
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
				//DrawMapDebug();
				
				DrawScore();
				DrawGameClock();

				DrawPlayers();
				DrawBall(&gs.ball);
				
				DrawPartics();
				DrawEndGameWinningText();

				//test debug
				if (gs.mapIndex == MAP_BIG_S) {
					PointLight();
					DrawMapDebug();
				}

				break; //end SCREEN_STATE_GAME

			case SCREEN_STATE_REWIND:
				//this is mostly for debug
				
				//step
				RewindStep();
				
				//draw
				ClearScreenSoildColor();
				DrawMapDebug();
				
				DrawScore();
				DrawGameClock();

				DrawPlayers();
				DrawPlayersDebug();
				
				DrawBall(&gs.ball);
				DrawBallDebug(&gs.ball);
				
				DrawPartics();
				

				break; //end SCREEN_STATE_REWIND

			case SCREEN_STATE_INSTANT_REPLAY:
				//draw
				DrawBackground(gs.mapIndex);
				DrawMapDebug();
				
				DrawScore();
				DrawGameClock();

				DrawPlayers();
				DrawBall(&gs.ball);
				
				DrawPartics();
				DrawEndGameWinningText();

				//load next state in replay
				ReplayScreenStep();
				break; //end SCREEN_STATE_INSTANT_REPLAY

			case SCREEN_STATE_GAME_PAUSED:
				SpriteTimerTick();
				DrawBackground(gs.mapIndex);
				DrawPlayers();
				//DrawPartics();
				break;

			default:
				#ifdef NDEBUG
				break;
				#else
				printf("bad screen state\n");
				assert(false);
				#endif
			}
			//end of screen state switch case
		
			//draw fps counter
			if (drawFpsCounter) {
				DrawTextNumberAppend(FPSDISP_X, FPSDISP_Y, false, FPS_TEXT" ", FPScounterMs());
			}

			//screen log draw
			LogTextScreenTickTimers();
			LogTextScreenDraw();

		}//end of game logic

		//drawing
		DrawRenderToScreen();

	} while (!quitGame);
	//end of main game loop

	//exit
	CloseJoypads();
	CloseSound();
	ShutdownWindow();
	return EXIT_SUCCESS;
}