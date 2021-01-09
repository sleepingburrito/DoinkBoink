#pragma once
//#include <stdint.h>

//Game Info
#define GAME_NAME "DoinkBoink"
#define GAME_VS "0.1"

//file
#ifdef _WIN32
	#define FILE_DIR "\\" //windows dir
#else
	#define FILE_DIR "/" //linux dir
#endif

//graphics
#define BASE_RES_WIDTH 960
#define BASE_RES_WIDTH_HALF (BASE_RES_WIDTH / 2)
#define BASE_RES_HEIGHT 540 
#define BASE_RES_HEIGHT_HALF (BASE_RES_HEIGHT / 2) 

#define SPRITE_WIDTH 64
#define SPRITE_WIDTH_HALF (SPRITE_WIDTH / 2)
#define SPRITE_WIDTH_FORTH (SPRITE_WIDTH_HALF / 2)

#define SPRITE_HEIGHT SPRITE_WIDTH
#define SPRITE_HEIGHT_HALF (SPRITE_HEIGHT / 2)
#define SPRITE_HEIGHT_FORTH (SPRITE_HEIGHT_HALF / 2)

#define SPRITE_TEXTSMALL_WIDTH 16
#define SPRITE_TEXTSMALL_HEIGHT SPRITE_TEXTSMALL_WIDTH

#define FPSDISP_X (150)
#define FPSDISP_Y (5)

#define BACKGROUND_SHAKE_START_RATE 30 //how much shake to start
#define BACKGROUND_SHAKE_DECAY_RATE 1 //how fast it will decay
#define BACKGROUND_SHAKE_DIV 10 //reduce shake effect

#define SPRITE_FILE_BASE "graphics" FILE_DIR

#define SPRITE_FILE SPRITE_FILE_BASE "sprites.png"
#define SPRITE_TEXTBIG_FILE SPRITE_FILE_BASE "fontBig.png"
#define SPRITE_TEXTSMALL_FILE SPRITE_FILE_BASE "fontSmall.png"

#define BACKGROUND_FILE SPRITE_FILE_BASE "Backgrounds.png"


//sound
#define START_SOUND_EFFECTS_VOLUME INT8_MAX
#define SEFFECTS_BALL_MIN_SPEED 40
#define SEFFECTS_BALL_SOUND_COUNT 3

#define SOUND_FILE_BASE "sound" FILE_DIR

//effects
#define SOUND_EFFECTS_BASE SOUND_FILE_BASE FILE_DIR "effects" FILE_DIR
#define EFFECTS_TEST_FILE SOUND_EFFECTS_BASE "TestBoop.flac"
#define EFFECTS_BELLRING_FILE SOUND_EFFECTS_BASE "BellRing.flac"
#define EFFECTS_BOOM_FILE SOUND_EFFECTS_BASE "Boom.flac"
#define EFFECTS_BOUNCE1_FILE SOUND_EFFECTS_BASE "Bounce1.flac"
#define EFFECTS_BOUNCE2_FILE SOUND_EFFECTS_BASE "Bounce2.flac"
#define EFFECTS_BOUNCE3_FILE SOUND_EFFECTS_BASE "Bounce3.flac"
#define EFFECTS_CLICK_FILE SOUND_EFFECTS_BASE "Click.flac"
#define EFFECTS_CLICKCLOCK_FILE SOUND_EFFECTS_BASE "ClickClock.flac"
#define EFFECTS_DULESQEEK_FILE SOUND_EFFECTS_BASE "DuleSqeek.flac"
#define EFFECTS_HITBONG_FILE SOUND_EFFECTS_BASE "HitBong.flac"
#define EFFECTS_PITTERPATTER_FILE SOUND_EFFECTS_BASE "PitterPatter.flac"
#define EFFECTS_QUICKPITTERPATTER_FILE SOUND_EFFECTS_BASE "QuickPitterPatter.flac"
#define EFFECTS_REVERSEDHIT_FILE SOUND_EFFECTS_BASE "ReversedHit.flac"
#define EFFECTS_SQEEKIN_FILE SOUND_EFFECTS_BASE "SqeekIn.flac"
#define EFFECTS_SQEEKOUTFAST_FILE SOUND_EFFECTS_BASE "SqeekOutFast.flac"
#define EFFECTS_SQEEKSLOW_FILE SOUND_EFFECTS_BASE "SqeekSlow.flac"
#define EFFECTS_WORBLE_FILE SOUND_EFFECTS_BASE "Worble.flac"
#define EFFECTS_AIRHORN_FILE SOUND_EFFECTS_BASE "AirHorn.flac"

//music
#define MUSIC_FAID_SPEED 6 //keep lower than MUSIC_FAID_CUTOFF
#define MUSIC_FAID_CUTOFF 10
#define MUSIC_MAX_VOLUME ((int8_t)(START_SOUND_EFFECTS_VOLUME * 0.8))

#define SOUND_MUSIC_BASE SOUND_FILE_BASE FILE_DIR "music" FILE_DIR
#define MUSIC_01_FILE SOUND_MUSIC_BASE "Song01.ogg"
#define MUSIC_02_FILE SOUND_MUSIC_BASE "Song02.ogg"
#define MUSIC_03_FILE SOUND_MUSIC_BASE "Song03.ogg"
#define MUSIC_04_FILE SOUND_MUSIC_BASE "Song04.ogg"
#define MUSIC_05_FILE SOUND_MUSIC_BASE "Song05.ogg"
#define MUSIC_06_FILE SOUND_MUSIC_BASE "Song06.ogg"
#define MUSIC_07_FILE SOUND_MUSIC_BASE "Song07.ogg"
#define MUSIC_08_FILE SOUND_MUSIC_BASE "Song08.ogg"
#define MUSIC_09_FILE SOUND_MUSIC_BASE "Song09.ogg"
#define MUSIC_10_FILE SOUND_MUSIC_BASE "Song10.ogg"
#define MUSIC_11_FILE SOUND_MUSIC_BASE "Song11.ogg"


enum SOUNDS_EFFECTS {
	SOUND_EFFECT_TEST,
	SOUND_EFFECT_BELLRING,
	SOUND_EFFECT_BOOM,
	SOUND_EFFECT_BOUNCE1,
	SOUND_EFFECT_BOUNCE2,
	SOUND_EFFECT_BOUNCE3,
	SOUND_EFFECT_CLICK,
	SOUND_EFFECT_CLICKCLOCK,
	SOUND_EFFECT_DULESQEEK,
	SOUND_EFFECT_HITBONG,
	SOUND_EFFECT_PITTERPATTER,
	SOUND_EFFECT_QUICKPITTERPATTER,
	SOUND_EFFECT_REVERSEDHIT,
	SOUND_EFFECT_SQEEKIN,
	SOUND_EFFECT_SQEEKOUTFAST,
	SOUND_EFFECT_SQEEKSLOW,
	SOUND_EFFECT_WORBLE,
	SOUND_EFFECT_AIRHORN,

	SOUNDS_EFFECTS_COUNT,
	SOUND_EFFECTS_NON
};

enum MUSIC_ENUM {
	SOUND_MUSIC_01,
	SOUND_MUSIC_02,
	SOUND_MUSIC_03,
	SOUND_MUSIC_04,
	SOUND_MUSIC_05,
	SOUND_MUSIC_06,
	SOUND_MUSIC_07,
	SOUND_MUSIC_08,
	SOUND_MUSIC_09,
	SOUND_MUSIC_10,
	SOUND_MUSIC_11,

	MUSIC_COUNT,
	MUSIC_NON
};

//end of sound

//timing
#define TARGET_FRAME_RATE 60 //in FPS
#define MS_TILL_UPDATE (1000 / TARGET_FRAME_RATE - 1) //game logic update rate, this rate will be used (if vsynce is slower that will be used). -1 to round down for headroom
#define MS_TIME_SHIFT_MASK 3 //used for setting the max slowdown when adjusting game speed

#define WATCHDOD_MASK 511 //contolles the rate the watchdog send a update (inframes)
#define WATCHDOG_MSG "WATCHDOG PING"


//fixed point
#define FIX_POINT_OFFSET 3

//math
#define FLAG_LENGTH 8 //bits in a flag
#define FLAG_HALF_LENGTH (FLAG_LENGTH >> 1) 

//game physics
#define MAX_SPEED_FIXPOINT (UINT8_MAX >> FIX_POINT_OFFSET)
#define MAX_SPEED UINT8_MAX
#define GRAVITY 9
//for (double i = 0; i < 256; ++i) printf("%d,", (int)(i * 0.99));
//955
const uint8_t GROUND_FRICTION[UINT8_MAX + 1] = { 0, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243 };
//985
const uint8_t AIR_FRICTION[UINT8_MAX + 1] = { 0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251 };
//86
const uint8_t BRAKES_FRICTION[UINT8_MAX + 1] = { 0,0,0,2,3,4,5,6,6,7,8,9,10,11,12,12,13,14,15,16,17,18,18,19,20,21,22,23,24,24,25,26,27,28,29,30,30,31,32,33,34,35,36,36,37,38,39,40,41,42,43,43,44,45,46,47,48,49,49,50,51,52,53,54,55,55,56,57,58,59,60,61,61,62,63,64,65,66,67,67,68,69,70,71,72,73,73,74,75,76,77,78,79,79,80,81,82,83,84,85,86,86,87,88,89,90,91,92,92,93,94,95,96,97,98,98,99,100,101,102,103,104,104,105,106,107,108,109,110,110,111,112,113,114,115,116,116,117,118,119,120,121,122,122,123,124,125,126,127,128,129,129,130,131,132,133,134,135,135,136,137,138,139,140,141,141,142,143,144,145,146,147,147,148,149,150,151,152,153,153,154,155,156,157,158,159,159,160,161,162,163,164,165,165,166,167,168,169,170,171,172,172,173,174,175,176,177,178,178,179,180,181,182,183,184,184,185,186,187,188,189,190,190,191,192,193,194,195,196,196,197,198,199,200,201,202,202,203,204,205,206,207,208,208,209,210,211,212,213,214,215,215,216,217,218,219 };
//707 for 45 degreese
const uint8_t TRUST_ANGLE[UINT8_MAX + 1] = { 1, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 21, 22, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 38, 39, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 48, 48, 49, 50, 50, 51, 52, 53, 53, 54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 74, 75, 76, 77, 77, 78, 79, 79, 80, 81, 82, 82, 83, 84, 84, 85, 86, 86, 87, 88, 89, 89, 90, 91, 91, 92, 93, 94, 94, 95, 96, 96, 97, 98, 98, 99, 100, 101, 101, 102, 103, 103, 104, 105, 106, 106, 107, 108, 108, 109, 110, 110, 111, 112, 113, 113, 114, 115, 115, 116, 117, 118, 118, 119, 120, 120, 121, 122, 123, 123, 124, 125, 125, 126, 127, 127, 128, 129, 130, 130, 131, 132, 132, 133, 134, 135, 135, 136, 137, 137, 138, 139, 139, 140, 141, 142, 142, 143, 144, 144, 145, 146, 147, 147, 148, 149, 149, 150, 151, 152, 152, 153, 154, 154, 155, 156, 156, 157, 158, 159, 159, 160, 161, 161, 162, 163, 164, 164, 165, 166, 166, 167, 168, 168, 169, 170, 171, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178, 179, 180 };
//rng
const uint8_t RNG_TABLE[UINT8_MAX + 1] = { 25, 223, 204, 22, 170, 81, 26, 16, 124, 255, 161, 162, 65, 42, 213, 162, 198, 231, 48, 131, 110, 148, 79, 18, 139, 77, 17, 125, 203, 138, 118, 40, 251, 138, 170, 3, 11, 38, 185, 84, 74, 68, 163, 142, 241, 73, 143, 17, 236, 177, 150, 138, 162, 110, 107, 231, 103, 203, 73, 202, 138, 159, 184, 115, 181, 194, 198, 207, 94, 243, 238, 66, 10, 223, 164, 147, 132, 41, 108, 109, 212, 90, 238, 144, 143, 79, 204, 197, 42, 145, 96, 56, 90, 164, 191, 88, 235, 193, 214, 15, 249, 235, 205, 107, 231, 164, 178, 226, 253, 19, 5, 166, 36, 106, 171, 11, 203, 26, 129, 151, 43, 58, 221, 16, 23, 170, 236, 158, 111, 113, 145, 126, 199, 231, 143, 58, 72, 13, 127, 55, 2, 117, 58, 130, 252, 199, 78, 31, 101, 13, 201, 100, 143, 54, 107, 172, 153, 207, 10, 216, 56, 88, 80, 110, 146, 149, 218, 126, 178, 31, 71, 125, 18, 158, 156, 45, 36, 55, 87, 42, 130, 228, 166, 121, 93, 39, 38, 94, 127, 251, 107, 111, 214, 132, 150, 188, 151, 191, 17, 5, 133, 10, 240, 142, 223, 179, 207, 6, 10, 220, 39, 31, 136, 130, 24, 51, 91, 130, 34, 201, 137, 209, 92, 60, 91, 34, 185, 218, 218, 64, 170, 28, 88, 183, 253, 151, 232, 95, 161, 194, 186, 9, 53, 219, 210, 132, 98, 74, 118, 40, 12, 171, 57, 130, 191, 219 };
//sin
const int8_t SIN_TABLE[UINT8_MAX + 1] = { 0,3,6,9,12,15,18,21,25,28,31,34,37,40,43,46,49,52,54,57,60,63,66,68,71,73,76,79,81,83,86,88,90,92,95,97,99,101,103,104,106,108,110,111,113,114,115,117,118,119,120,121,122,123,124,125,125,126,126,127,127,127,127,127,127,127,127,127,127,126,126,125,125,124,123,123,122,121,120,119,117,116,115,113,112,110,109,107,105,104,102,100,98,96,94,91,89,87,85,82,80,77,75,72,70,67,64,61,59,56,53,50,47,44,41,38,35,32,29,26,23,20,17,14,11,7,4,1,-1,-4,-7,-11,-14,-17,-20,-23,-26,-29,-32,-35,-38,-41,-44,-47,-50,-53,-56,-59,-61,-64,-67,-70,-72,-75,-77,-80,-82,-85,-87,-89,-91,-94,-96,-98,-100,-102,-104,-105,-107,-109,-110,-112,-113,-115,-116,-117,-119,-120,-121,-122,-123,-123,-124,-125,-125,-126,-126,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-126,-126,-125,-125,-124,-123,-122,-121,-120,-119,-118,-117,-115,-114,-113,-111,-110,-108,-106,-104,-103,-101,-99,-97,-95,-92,-90,-88,-86,-83,-81,-79,-76,-73,-71,-68,-66,-63,-60,-57,-54,-52,-49,-46,-43,-40,-37,-34,-31,-28,-25,-21,-18,-15,-12,-9,-6,-3 };

//physics masks
enum physicsMasks {
	PHYSICS_BOUNCE,
	PHYSICS_ONGROUND, //set when player was in ground last frame
	PHYSICS_IN_WALL, //set when player hits any wall in all 4 dirs
	PHYSICS_IN_HORIZONTAL_WALL, //set if hit wall on x axis
	PHYSICS_BRAKES,
	PHYSICS_DISABLE_GRAVITY
};


//player settings
#define PLAYER_ACC_GROUND 4
#define PLAYER_ACC_GROUND_RUNNING 6
#define PLAYER_ACC_AIR 3
#define PLAYER_ACC_WALL_CLIMB PLAYER_ACC_GROUND_RUNNING

#define PLAYER_JUMP 110
#define PLAYER_DUCK_JUMP 160
#define PLAYER_FALL_BOOST 23

#define PLAYER_WALL_JUMPH 80
#define PLAYER_WALL_JUMPV 80
#define PLAYER_WALL_JUMP_TIMER 8 //number of frames away from a wall you can still do a wall jump in

#define PLAYER_DOUBLE_JUMP 50
#define PLAYER_FLASH_JUMPH 43
#define PLAYER_FLASH_JUMPV 13


#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 64
#define PLAYER_HEIGHT_DUCKING_SHIFT 2

//these spawn locaions are only used on symetrical maps, spawn is set in map data
#define PLAYER_ONE_START_X (BASE_RES_WIDTH / 4 - SPRITE_WIDTH_HALF)
#define PLAYER_ONE_START_Y 330

#define PLAYER_TWO_START_X (BASE_RES_WIDTH / 4 * 3 - SPRITE_WIDTH_HALF)
#define PLAYER_TWO_START_Y 330
//end of spawn locaions

#define PLAYER_STUN_START 100

//trow speed test
#define PLAYER_TROW_HIGHT_OFFSET (PLAYER_HEIGHT / 4)
#define PLAYER_CHARGE_MAX_TIME 20
#define PLAYER_CHARGE_BITSHIFTER 1 
#define PLAYER_CHARGE_STUN_RUNOFF 5 //little stun after attack
#define PLAYER_CHARGE_MIN_GRAVITY_TRUST 3 //ball keeps gravity if trust is smaller than this

//if you make this too big it will wrap around
#define BASE_TROW_MUTI 19 
#define BASE_TROW_MUTI_PURE_UP 25 //used only for pure up trows
#define PLAYER_TROW_RECOIL_SHIFT 2
#define TROW_STUN 25 //after a trow you cant attack for this long (but still catch)

#define PLAYER_HEAD_BALL_BOUNCE 10 //kinda just for fun, can bouce ball off head

#define PLAYER_CATCH_TIME 14
#define PLAYER_CATCH_TIME_STUN (PLAYER_CATCH_TIME + 10)
#define PLAYER_CATCH_INFLATE 20
#define PLAYER_DODGE_PROTECTION_TIME 24

#define PLAYER_DODGE_TIME 17
#define PLAYER_DODGE_COOLDOWN_TIME (PLAYER_DODGE_TIME + 20)
#define PLAYER_DODGE_SPEED_BOOST 58
#define PLAYER_DODGE_SPEED_BOOST_ANGLE (uint8_t)(PLAYER_DODGE_SPEED_BOOST * 0.707)

#define PLAYER_BALL_TOO_FAST 100 //this value is for if you have trowing or trowing rocket combined
#define PLAYER_LOWSPEED_TIME 30 //downt switch allagnce too fast when trown

#define PLAYER_HIT_TIME 100 //cant attack and invensable, but can still move (if you get hit)
#define PLAYER_SCORE_PAUSE_GAME 18 //in frames for dramtic effect, pauses game play
#define PLAYER_DEAD_BOUNCE_TIME 35 //you bounce during this time

#define PLAYER_MAX_SCORE 5 //match ends after somone dies more than this

#define PLAYER_GAME_CLOCK_MAX (90 * TARGET_FRAME_RATE)

#define PLAYER_BLINK_RATE 0b00000100
#define PLAYER1_RED_DEBUG 0xFF
#define PLAYER1_GREEN_DEBUG 0x0
#define PLAYER1_BLUE_DEBUG 0xFF

#define PLAYER2_RED_DEBUG 0x00
#define PLAYER2_GREEN_DEBUG 0XFF
#define PLAYER2_BLUE_DEBUG 0xFF

#define PLAYER_NOSE_COLOR_RED 0x00
#define PLAYER_NOSE_COLOR_GREEN 0xFF
#define PLAYER_NOSE_COLOR_BLUE 0x00

#define PLAYER1_DEBUG_SCORE_X 10
#define PLAYER1_DEBUG_SCORE_Y 10

#define PLAYER2_DEBUG_SCORE_X (BASE_RES_WIDTH >> 1)
#define PLAYER2_DEBUG_SCORE_Y 10

#define PLAYER1_DRAW_SCORE_X 1
#define PLAYER1_DRAW_SCORE_Y 5
#define PLAYER2_DRAW_SCORE_X (BASE_RES_WIDTH - SPRITE_WIDTH * 3)
#define PLAYER2_DRAW_SCORE_Y PLAYER1_DRAW_SCORE_Y

enum whichPlayerFlag
{
	PLAYER_ONE,
	PLAYER_TWO,
	PLAYER_COUNT
};

enum playerMasks
{
	PLAYER_SECOND, //this flag is set if your second player, else your first
	PLAYER_FACING_RIGHT,
	PLAYER_DUCKING,
	PLAYER_TROW_LEFT,
	PLAYER_TROW_UP,
	PLAYER_TROW_H,
	PLAYER_TROW_V,
	PLAYER_HAS_DOUBLE_JUMP
};


enum playerTimersIndex {
	PLAYER_STUN_TIMER, //cant io player
	PLAYER_INVISIBILITY_TIMER,
	
	PLAYER_CATCH_TIMER,

	PLAYER_BLINK_TIMER, //display only
	PLAYER_SOLID_TIMER, //display only

	PLAYER_DODGE_TIMER,
	PLAYER_DODGE_TIMER_COOLDOWN,

	PLAYER_CHARGE_TROW_TIMER,
	PLAYER_STEAL_PROTECTION_TIMER,

	PLAYER_CANT_ATTACK_TIMER,
	PLAYER_BOUNCH_TIMER,
	PLAYER_SPAWN_TIMER, //used to denote things that can't be done while the player is spawning
	
	PLAYER_WALLJUMP_TIMER,

	//AI
	PLAYER_AI_NEW_WALK_LOC,
	PLAYER_AI_ATTACK_TIMER,

	PLAYER_TIMER_COUNT
};


//AI
//======
enum playerAiMasks {
	AI_ENABLED, //normal AI
	AI_FETCH //cetchs the ball and brings it back to you
};

#define AI_MAP_DEBUG_PLATX_LEFT 600 //helps the AI get around the platfrom on MAP_DEBUG
#define AI_MAP_DEBUG_PLATX_RIGHT TO_FIXPOINT(BASE_RES_WIDTH - 100)
#define AI_MAP_DEBUG_PLATY 2000

#define AI_DISTANCE_RUN 500 //stop running when this close to goal
// AI_DISTANCE_NEW_WALK_LOC 300

//#define AI_MAP_RNG_START 50
//#define AI_MAP_RNG_MASK_LENGTH 511

//miss a catch (smaller number more missing)
#define AI_MISS_RATE_EASY 50
#define AI_MISS_RATE_MEDIUM 128 
#define AI_MISS_RATE_HARD 200

//timer trow rate (bigger mask longer times between trows)
#define AI_TROW_TIMER_MASK_EASY 254
#define AI_TROW_TIMER_MASK_MEDIUM 127
#define AI_TROW_TIMER_MASK_HARD AI_TROW_TIMER_MASK_MEDIUM

#define AI_DODGE_RATE 100
#define AI_DISTANCE_DODGE 300

#define AI_RNG_KEY 5

//what AI gets set when players are INIT
enum AI_GLOB_SETTING
{
	AI_SET_OFF,
	AI_SET_EASY,
	AI_SET_MEDIUM,
	AI_SET_HARD,
	AI_SET_FETCH,

	AI_SETTINGS_COUNT
};

//
//AI end

//==instant replay==
#define REPLAY_START_IN 40 //how long till the reaply starts once called
#define REPLAY_FRAME_START 120 //keep under UIN8_MAX, number of frames to start back in recoding

#define REPLAY_TEXT_X 350
#define REPLAY_TEXT_Y (BASE_RES_HEIGHT / 2 - SPRITE_HEIGHT_HALF)
#define REPLAY_BLINK_MASK 8
//



//obj general
enum objFacingFlag {
	OBJ_FACING_LEFT,
	OBJ_FACING_RIGHT
};

enum directionState {
	DIR_NON,
	DIR_UP,
	DIR_DOWN,
	DIR_RIGHT,
	DIR_LEFT
};

enum speedIndex
{
	SPEED_UP_INDEX,
	SPEED_DOWN_INDEX,
	SPEED_LEFT_INDEX,
	SPEED_RIGHT_INDEX,
	SPEED_INDEX_COUNT
};

enum axisIndex {
	X_AXIS,
	Y_AXIS,
	AXIS_COUNT
};

enum RngMaskOffsets
{
	//used with rng mask to make range 0-RNG_MASK_X
	//8bit
	RNG_MASK_0,
	RNG_MASK_1,
	RNG_MASK_3,
	RNG_MASK_7,
	RNG_MASK_15,
	RNG_MASK_31,
	RNG_MASK_63,
	RNG_MASK_127,
	//16 bit
	RNG_MASK_255,
	RNG_MASK_511,
	RNG_MASK_1023
};


//Game Pad
//note: can only have 8 buttons max for main gameplay
enum padMasks
{
	PAD_UP,
	PAD_DOWN,
	PAD_LEFT,
	PAD_RIGHT,
	PAD_JUMP,
	PAD_ACTION,
	PAD_DODGE,
	PAD_RUN,
	PAD_START,
	PAD_BUTTON_COUNT
};

#define IO_STATE_COUNT_MAX (PLAYER_COUNT * PAD_STATE_COUNT)

//game io
#define P1_DEADZONE 11000
#define P1_DEADZONE_DOWN 12000

//player 1 keys
#define P1_PAD_AXIS_X 0
#define P1_PAD_AXIS_X_ALT P1_PAD_AXIS_X

#define P1_PAD_AXIS_Y 1
#define P1_PAD_AXIS_Y_ALT P1_PAD_AXIS_Y 

#define P1_KEY_UP SDL_SCANCODE_W
#define P1_KEY_DOWN SDL_SCANCODE_S
#define P1_KEY_LEFT SDL_SCANCODE_A
#define P1_KEY_RIGHT SDL_SCANCODE_D

#define P1_PAD_BUTTON_JMP 1
#define P1_PAD_BUTTON_JMP_ALT P1_PAD_BUTTON_JMP
#define P1_KEY_JMP SDL_SCANCODE_C

#define P1_PAD_BUTTON_ACTION 2 //trow / catch
#define P1_PAD_BUTTON_ACTION_ALT P1_PAD_BUTTON_ACTION
#define P1_KEY_ACTION SDL_SCANCODE_V

#define P1_PAD_BUTTON_RUN 0
#define P1_PAD_BUTTON_RUN_ALT P1_PAD_BUTTON_RUN
#define P1_KEY_RUN SDL_SCANCODE_B

#define P1_PAD_BUTTON_DODGE 5 //dodge
#define P1_PAD_BUTTON_DODGE_ALT 3 
#define P1_KEY_DODGE SDL_SCANCODE_N
//end player 1 keys

//player 2 keys
#define P2_DEADZONE P1_DEADZONE
#define P2_DEADZONE_DOWN P1_DEADZONE_DOWN

#define P2_PAD_AXIS_X 0
#define P2_PAD_AXIS_X_ALT P1_PAD_AXIS_X

#define P2_PAD_AXIS_Y 1
#define P2_PAD_AXIS_Y_ALT P1_PAD_AXIS_Y 

#define P2_KEY_UP SDL_SCANCODE_UP
#define P2_KEY_DOWN SDL_SCANCODE_DOWN
#define P2_KEY_LEFT SDL_SCANCODE_LEFT
#define P2_KEY_RIGHT SDL_SCANCODE_RIGHT

#define P2_PAD_BUTTON_JMP P1_PAD_BUTTON_JMP
#define P2_PAD_BUTTON_JMP_ALT P2_PAD_BUTTON_JMP
#define P2_KEY_JMP SDL_SCANCODE_H

#define P2_PAD_BUTTON_ACTION P1_PAD_BUTTON_ACTION
#define P2_PAD_BUTTON_ACTION_ALT P2_PAD_BUTTON_ACTION
#define P2_KEY_ACTION SDL_SCANCODE_J

#define P2_PAD_BUTTON_RUN P1_PAD_BUTTON_RUN
#define P2_PAD_BUTTON_RUN_ALT P2_PAD_BUTTON_RUN
#define P2_KEY_RUN SDL_SCANCODE_K

#define P2_PAD_BUTTON_DODGE P1_PAD_BUTTON_DODGE
#define P2_PAD_BUTTON_DODGE_ALT P1_PAD_BUTTON_DODGE_ALT
#define P2_KEY_DODGE SDL_SCANCODE_L
//end player 2 keys

//keys global
#define PAD_DEBUG_START 6 //(select) hold this down to allow any debugging
#define KEY_DEBUG_START SDL_SCANCODE_0

#define PAD_DEBUG_SWAP 5 //swaps players contollors, right bumper
#define KEY_DEBUG_SWAP SDL_SCANCODE_1

#define PAD_DEBUG_REINIT 8 //left analog click in
#define KEY_DEBUG_REINIT SDL_SCANCODE_2

#define PAD_DEBUG_REWIND 9 //click in right joystick
#define KEY_DEBUG_REWIND SDL_SCANCODE_3

#define PAD_DEBUG_CHANGE_MAP 4 //left bumper (if a contollr gets disconnted run this)
#define KEY_DEBUG_CHANGE_MAP SDL_SCANCODE_4

#define PAD_PAUSE 7 //start button
#define KEY_PAUSE SDL_SCANCODE_RETURN

//end of keys global


enum padStatesIndex{
	PAD_STATE_HELD,
	PAD_STATE_TAP,
	PAD_DELTA_BUFFER,

	PAD_STATE_COUNT
};


//game screen state
enum screenStateIndex {
	SCREEN_STATE_MAIN_MENU,
	SCREEN_STATE_GAME,
	SCREEN_STATE_REWIND,
	SCREEN_STATE_INSTANT_REPLAY,
	SCREEN_STATE_GAME_PAUSED,
	
	SCREEN_STATE_COUNT,
	SCREEN_STATE_NON
};

#define BLOCK_ALL_IO_TIME 18 //in frames, use to debouce contorlls

//End of Game Pad

//Ball
enum ballMasks {
	BALL_ON_PLAYER,//if player owned
	BALL_ON_PLAYER2,//if player 2, if not assume player 1
	BALL_INSIDE_PLAYER,
	BALL_CHARGED, //if this bit is set the ball may be tossed at max charge (for parry)
	BALL_NEUTRAL, //if set cant hurt player, else its moving fast enough to hurt player
	BALL_TOO_FAST
};

//these spawn locaions are only used on symetrical maps
#define BALL1_START_X (BASE_RES_WIDTH / 2 - SPRITE_WIDTH_HALF)
#define BALL1_START_Y 200
//end of spawn

#define BALL_SPRITE_WIDTH 32
#define BALL_SPRITE_HEIGTH BALL_SPRITE_WIDTH

#define BALL_TIMER_PLAYER_IGNORE_BOUNCE 5
#define BALL_PARRY_GET_TIME 30
#define BALL_PARRY_TIMEOUT 200

#define BALL_NEUTRAL_DEBUG_R 128
#define BALL_NEUTRAL_DEBUG_G 128
#define BALL_NEUTRAL_DEBUG_B 128

enum ballTimerIndex {
	BALL_PLAYER_BOUNCE_IGNOR,
	BALL_LOWSPEED_IGNOR,
	BALL_PARRY_TIMER,
	BALL_PARRY_TIMER_TIMEOUT,
	BALL_TIMER_COUNT
};



//world
enum worldTimersIndex {
	WOULD_PAUSE_TIMER,
	WOLD_TIMER_COUNT
};



//sprites
enum spriteIndexes {
	SPRITE_INDEX_BLANK,
	SPRITE_INDEX_BOX, 
	SPRITE_INDEX_SPARK,
	SPRITE_INDEX_SMALLBALL,
	SPRITE_INDEX_BALLSTILL,
	SPRITE_INDEX_BALLHRZ,
	SPRITE_INDEX_BALLVERT,
	SPRITE_INDEX_GROUNDSMOKE, //not in use
	SPRITE_INDEX_SMOKERING,
	SPRITE_INDEX_SMALLSMOKE,
	SPRITE_INDEX_BIGSMOKE,
	SPRITE_INDEX_PLAYERRUN,
	SPRITE_INDEX_PLAYERWALK,
	SPRITE_INDEX_PLAYERCATCH,
	SPRITE_INDEX_PLAYERUP,
	SPRITE_INDEX_PLAYERDOWN,//not in use
	SPRITE_INDEX_PLAYERFLAP, //used for falling
	SPRITE_INDEX_PLAYERSTAND,
	SPRITE_INDEX_PLAYERDUCK,
	SPRITE_INDEX_PLAYERCRAW,
	SPRITE_INDEX_PLAYERSHEILD,
	SPRITE_INDEX_PLAYERCLIMPUP,
	SPRITE_INDEX_PLAYERWALLHOLD,
	SPRITE_INDEX_PLAYERCLIMBDOWN,
	SPRITE_INDEX_PLAYERHEADBOUNCH, //used as winning pose
	SPRITE_INDEX_PLAYERTROW,
	SPRITE_INDEX_PLAYERDMG,
	SPRITE_INDEX_PLAYER_STEAL,

	SPRITE_INDEX_COUNT
};


#define SPRITE_SPEED_NORMAL 3
#define SPRITE_SPEED_FAST 2

//Particles
#define PARTICLES_MAX 100



//map
#define MAP_MAX_BLOCKS 5

enum MapIndexes {
	MAP_DEBUG, //de_bug, hehe
	MAP_EMPY,
	MAP_BIG_S,
	
	MAP_COUNT
};

enum MapSpawnIndexes {
	MAP_SPAWN_PLAYER1_X,
	MAP_SPAWN_PLAYER1_Y,
	MAP_SPAWN_PLAYER2_X,
	MAP_SPAWN_PLAYER2_Y,
	MAP_SPAWN_BALL_X,
	MAP_SPAWN_BALL_Y,

	MAP_SPAWN_COUNT
};

//map background image data

//map data below
//note walls must be ticker than max speed and walls should not overlap
//player1 x, y
//player2 x, y
//ball x, y
//x, y, h, w

//current
const uint16_t mapDebug[] = { //MAP_DEBUG
	PLAYER_ONE_START_X , PLAYER_ONE_START_Y - 200,
	PLAYER_TWO_START_X , PLAYER_TWO_START_Y - 200,
	BALL1_START_X, BALL1_START_Y,
	7, 474, 33, 947,
	5, 7, 457, 33,
	918, 13, 454, 33,
	56, 6, 33, 845,
	230, 280, 33, 480,
};

const uint16_t mapEmpy[] = {
	PLAYER_ONE_START_X, PLAYER_ONE_START_Y,
	PLAYER_ONE_START_X, PLAYER_ONE_START_X,
	PLAYER_TWO_START_X, 100,
	//4 walls
	3, 3, 33, 951,
	2, 44, 423, 33,
	3, 474, 33, 951,
	920, 42, 425, 33
};

const uint16_t mapBigS[] = {
	//spawns
	PLAYER_ONE_START_X + 100, PLAYER_ONE_START_Y,
	PLAYER_TWO_START_X - 100, PLAYER_TWO_START_Y,
	BALL1_START_X + 15, 35 + 33 + 1,
	//walls
	45, 35, 33, 870, //celing
	45, 454, 43, 870, //ground
	2, 44, 423, 33, //walls
	920, 42, 425, 33,
	//inside
	462, 135, 215, 32
};


//=== game text ===
//#define REPLAY_TEXT "INSTANT REPLAY"

#define FPS_TEXT "FPS"

#define END_TEXT_TIE "TIE"
#define END_TEXT_WIN "WIN TO PLAYER "
#define END_TEXT_TIMEOUT_WIN "TIMOUT WIN PLAYER "
#define END_TEXT_PERFECT "PERFECT GAME PLAYER "

#define GAME_CLOCK_X 300
#define GAME_CLOCK_Y 10

#define LOADING_TEXT_BASE "LOADING"
#define LOADING_TEXT_SPRITES LOADING_TEXT_BASE " SPRITES" 
#define LOADING_TEXT_SOUNDS  LOADING_TEXT_BASE " SOUNDS"
#define LOADING_TEXT_MUSIC  LOADING_TEXT_BASE " MUSIC"
#define LOADING_TEXT_X 20
#define LOADING_TEXT_Y LOADING_TEXT_X

#define TEXT_AI_OFF "AI OFF FOR PLAYER "
#define TEXT_AI_EASY "EASY AI FOR PLAYER "
#define TEXT_AI_MEDIUM "MEDIUM AI FOR PLAYER "
#define TEXT_AI_HARD "HARD AI FOR PLAYER "
#define TEXT_AI_FETCH "FETCH AI FOR PLAYER "

#define MAP_NAME_DEBUG "MAP DEBUG"
#define MAP_NAME_EMPTY "MAP EMPTY"
#define MAP_NAME_LINE "MAP CROSSING THE LINE"

//text log screen
#define TEXT_LOG_CHARS 255 //max chars in message
#define TEXT_LOG_LINES 5 //max lines on screen
#define TEXT_LOG_TIME 120 //frames on screen
#define TEXT_LOG_X 300
#define TEXT_LOG_Y 50
#define TEXT_LOG_Y_BUFFER 4