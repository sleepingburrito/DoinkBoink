#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <SDL_mixer.h>

//main.h
//
bool quitGame = false;
//game logic timing tools
uint8_t updateLogicRateTimeShifter = 0; //bit shift the game logic time to slow game down, pad.h handles this
bool drawFpsCounter = true; //safe set, this if you wish to have a fps counter on screen
Uint64 lastLogicUpdate = 0; //timer to limit game logic update rate
bool updateGameLogic = false; //used by timing tools in main to know when to update game logic, do not touch
uint32_t WatchDogFrameCount = 0; //handled by a watch dog
//settings
bool disableGameTimer = true; //set these to disable these features
bool disableGameScore = false; //set to disable game score
//menu state
gameScreenState screen; //id of what screen we should be on
gameScreenState screenStateSavePause; //where the screen was before it paused
//next map buffer
uint8_t newMapIndex = MAP_DEBUG;
//
//end of main.h


//GamePad.h
//
SDL_Joystick * joyPads[PLAYER_COUNT];
flags padIO[IO_STATE_COUNT_MAX]; //raw IO, always updated by GamePad.h
bool windowFullscreen = false; //used to toggle IO
uint8_t pauseIoTimer = 0; //takes away pad io for a set amount of time in frames, used as a denounce
//
//end of GamePad.h


//BaseCollision.h
//
//some of these are kinda redundant but just make coding easier
//put in your index and get out if you hit anything
playerBase* BallHitPlayer; //for ball
ballBase* playerHitBall[PLAYER_COUNT]; //for player, put player index and get if the ball hit
ballBase* playerHitBallInflate[PLAYER_COUNT]; //for player catching the ball
playerBase* playerHitPlayer[PLAYER_COUNT]; //if the players overlap
//
//BaseCollision.h


//Graphics.h
//
SDL_Window* window = NULL;
SDL_Surface* windowSurface = NULL;

SDL_Renderer* mainRenderer = NULL;
SDL_Texture* mainDrawTexture = NULL;

SDL_Texture* spriteTex = NULL; //holds the main game sprites
SDL_Texture* spriteTextTexBig = NULL; //holds the large font
SDL_Texture* spriteTextTexSmall = NULL; //small font
SDL_Texture* backgrounds = NULL;

uint16_t displayRefreshRate = 0; //read only, set when graphics are init

SDL_Rect lightBoxs[POINT_LIGHT_MAX_BOXES]; //used by point light for batch processing

textLogBase textLogBuffer[TEXT_LOG_LINES]; //used by the text log
//
//end of Graphics.h


//Sound.h
//all of below is handled by its own file
//
bool muteEffects = false; //Mute Sound Effects
bool playRandomMusic = false; //shuffle
bool muteMusic = false;
int8_t musicVolume = 0;
int8_t musicQueue = MUSIC_NON;
int8_t musicPlaying = MUSIC_NON;

Mix_Music* gameMusic[MUSIC_COUNT];
Mix_Chunk* gameSoundEffects[SOUNDS_EFFECTS_COUNT];

//used to signal player sounds (used in player drawing to find out if the sound should be played)
bool playersRunningSound = false; 
bool playersWalkingSound = false;
uint8_t spriteTimerSound = 1;

//ball sound effect offset
uint8_t ballSounIndexOffset = 0;
//
//end Sound.h


//main gameplay state
//
typedef struct {
	//map data
	boxWorldSpace map[MAP_MAX_BLOCKS];
	uint8_t mapBoxCount;
	uint8_t mapIndex; //read only, set by LoadMap
	uint16_t startSpawnMap[MAP_SPAWN_COUNT]; //Spawn locations of the ball/player

	//global timers
	uint16_t gameClock; //match timer
	timer worldTimers[WOLD_TIMER_COUNT];

	//graphics
	uint8_t spriteTimer;
	baseParticle particles[PARTICLES_MAX];
	uint8_t backgroundShakeRate; //set this to BACKGROUND_SHAKE_START_RATE to shake background

	//rng
	uint8_t rngSeed;

	//game objects
	playerBase players[PLAYER_COUNT];
	ballBase ball;

	//AI
	uint8_t settingsAi[PLAYER_COUNT]; //turn on the player AI with settings these
	
	//pad io used by gameplay helps with (for rewind or game pause)
	flags padIOReadOnly[IO_STATE_COUNT_MAX]; 

}mainState;
mainState gs;
//
//end of main gameplay state


//for Rewind.h
//
mainState tape[UINT8_MAX + 1];
uint8_t tapeFrame = 0;
uint8_t tapeFrameLast = 0; //go back to last frame when returning back to game play
uint8_t recordInitTime = 0; //only used for the first few seconds to make sure you filled the record buffer
//replay section
uint8_t replayStartTimer = 0; //also used to tell in other parts of the code that the game has ended
uint8_t replaySlowMo = 0;
bool autoMapSwitch = false; //may set, if set when the replay is over it will switch to the next map
//
//end of Rewind.h

#endif