#pragma once
#include <stdint.h>
#include <SDL_mixer.h>
#include "typedefs.h"


//main.h
//
bool quitGame = false;
//sdl events
SDL_Event event;
//game logic timing tools
uint8_t updateLogicRateTimeShifter = 0; //bitshit the game logic time to slow game down, set in padio
Uint64 lastLogicUpdate = 0;
bool updateGameLogic = false; //used by timing tools in main to know when to update game logic
bool drawFpsCounter = true; //safe set this if you wish to have a fps counter

uint32_t WatchDogFrameCount = 0; //monitored by a watch dog
//
//end of main.h

//IO.h
//
SDL_Joystick * joyPads[PLAYER_COUNT];
flags padIO[IO_STATE_COUNT_MAX]; //raw IO, always updated by IO.h
bool windowFullscreen = false;
uint8_t pauseIoTimer = 0; //takes away gloable controll for a set amout of time in frames
//
//end of io.h

//basecollision.h
//
//some of these are kinda redundent but just make coding easier
//put in your index and get out if you hit anything
ballBase* BallHitBall; 
playerBase* BallHitPlayer; //for ball, put ball index and get player
ballBase* playerHitBall[PLAYER_COUNT]; //for player, put player index and get which ball
ballBase* playerHitBallInflate[PLAYER_COUNT]; //for player catching the ball
playerBase* playerHitPlayer[PLAYER_COUNT];
//
//end of basecollision.h

//graphics.h
SDL_Window* window = NULL;
SDL_Surface* windowSurface = NULL;

SDL_Renderer* mainRenderer = NULL;
SDL_Texture* mainDrawTexture = NULL;

SDL_Texture* spriteTex = NULL;
SDL_Texture* spriteTextTexBig = NULL;
SDL_Texture* spriteTextTexSmall = NULL;
SDL_Texture* backgrounds = NULL;
//

//sound.h
//
bool muteEffects = false;
bool playRandomMusic = false;
bool muteMusic = false;
int8_t musicVolume = 0;
int8_t musicQueue = MUSIC_NON;
int8_t musicPlaying = MUSIC_NON;

Mix_Music* gameMusic[MUSIC_COUNT];
Mix_Chunk* gameSoundEffects[SOUNDS_EFFECTS_COUNT];

//used to signal player sounds (used in player drawing)
bool playersRunningSound = false; 
bool playersWalkingSound = false;
uint8_t spriteTimerSound = 1;
//ball sound effect offset
uint8_t ballSounIndexOffset = 0;
//end sound.h


//menu state
gameScreenState screen; //id of what screen we should be on
gameScreenState screenStateSavePause; //where the screen was befor it paused

//main gameplay state
typedef struct {
	boxWorldSpace map[MAP_MAX_BLOCKS];
	uint8_t mapBoxCount;
	uint8_t mapIndex; //read only, set by loadmap
	uint16_t startSpawnMap[MAP_SPAWN_COUNT];

	timer worldTimers[WOLD_TIMER_COUNT];
	uint8_t spriteTimer;

	uint8_t rngSeed;

	playerBase players[PLAYER_COUNT];
	ballBase ball;
	
	baseParticle particles[PARTICLES_MAX];

	//used by gameplay, can be paused (for rewind or game pause)
	flags padIOReadOnly[IO_STATE_COUNT_MAX]; 
}mainState;
mainState gs;

//for rewing.h
//
mainState tap[UINT8_MAX + 1];
uint8_t tapFrame = 0;
uint8_t tapFrameLast = 0; //go back to last frame when return back to game
//replay section
uint8_t replayStartTimer = 0;
uint8_t replaySlowMo = 0;
//
//end of rewind.h

//debug, used to switch maps
uint8_t newMapIndex = MAP_DEBUG;