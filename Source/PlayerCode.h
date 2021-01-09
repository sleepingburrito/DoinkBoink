#pragma once
#include "Tools.h"
#include <stdint.h>
#include <stdbool.h>
#include "GlobalState.h"
#include "Const.h"
#include "typedefs.h"
#include "BallCode.h"


playerBase InitPlayer(const uint8_t whichPlayer) {

	playerBase returnPlayer;

	ZeroOut((uint8_t*)&returnPlayer, sizeof(playerBase));
	//starting  timers
	returnPlayer.playerTimer[PLAYER_STUN_TIMER] = PLAYER_STUN_START;
	returnPlayer.playerTimer[PLAYER_BLINK_TIMER] = PLAYER_STUN_START;
	returnPlayer.playerTimer[PLAYER_SPAWN_TIMER] = PLAYER_STUN_START;
	returnPlayer.playerTimer[PLAYER_INVISIBILITY_TIMER] = PLAYER_STUN_START;
	//set if your player 2 (if not your player 1)
	if (whichPlayer == PLAYER_TWO) {
		//player 2
		FLAG_SET(returnPlayer.playerFlags, PLAYER_SECOND);
		//set player size and start loc
		returnPlayer.playerPhysics.postionWorldSpace = InitBox(
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER2_X]),
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER2_Y]),
			TO_FIXPOINT(PLAYER_HEIGHT),
			TO_FIXPOINT(PLAYER_WIDTH));
	}
	else {
		//player 1
		//set player size and start loc
		returnPlayer.playerPhysics.postionWorldSpace = InitBox(
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER1_X]),
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER1_Y]),
			TO_FIXPOINT(PLAYER_HEIGHT),
			TO_FIXPOINT(PLAYER_WIDTH));
	}

	//set the player to face the ball
	if (TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_BALL_X]) > returnPlayer.playerPhysics.postionWorldSpace.topLeft.x) {
		FLAG_SET(returnPlayer.playerFlags, PLAYER_FACING_RIGHT);
	}
	else {
		FLAG_ZERO(returnPlayer.playerFlags, PLAYER_FACING_RIGHT);
	}

	if (whichPlayer >= PLAYER_COUNT) {
#ifdef NDEBUG
#else
		printf("play index ob");
		assert(false);
#endif
	}

	//ai test
	//if (whichPlayer == PLAYER_TWO)
		//FLAG_SET(returnPlayer.AI, AI_ENABLED);

	return returnPlayer;
}

void InitPlayers(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		gs.players[i] = InitPlayer(i);
	}
}

//tools
bool PlayerFacingBall(const playerBase* const player, const ballBase* const ball) {
	if (player == NULL || ball == NULL) {
#ifdef NDEBUG
		return false;
#else
		printf("PlayerFacingBall Null ptr");
		assert(false);
#endif

	}

	bool ballGoingRight = false;
	if (ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX] > ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX]) {
		ballGoingRight = true;
	}

	return FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT) != ballGoingRight;
}

//player step
//
void PlayerStealBall(playerBase* const player) {
	//Check if in other player to steal their ball
	playerBase* const OtherPlayer = playerHitPlayer[FLAG_TEST(player->playerFlags, PLAYER_SECOND)];
	if (OtherPlayer != NULL) {
		//get the other players ball if they have one
		ballBase* const OtherBall = CheckPlayersBall(OtherPlayer->playerFlags);

		//check if player can steal
		if (player->playerTimer[PLAYER_CATCH_TIMER] != 0
			&& OtherPlayer->playerTimer[PLAYER_DODGE_TIMER] == 0
			&& OtherPlayer->playerTimer[PLAYER_STEAL_PROTECTION_TIMER] == 0
			&& OtherPlayer->playerTimer[PLAYER_CHARGE_TROW_TIMER] == 0
			&& OtherBall != NULL) {

			//let the player move again
			player->playerTimer[PLAYER_CATCH_TIMER] = 0;
			player->playerTimer[PLAYER_STUN_TIMER] = 0;

			//give player steal protecthing
			player->playerTimer[PLAYER_STEAL_PROTECTION_TIMER] = PLAYER_DODGE_PROTECTION_TIME;

			//tell the ball its mine
			BallGiveToPlayer(player->playerFlags, OtherBall);

			//sounds
			PlaySoundEffect(SOUND_EFFECT_DULESQEEK);
		}
	}
}

void PlayerBounce(playerBase* const player) {
	if (player->playerTimer[PLAYER_BOUNCH_TIMER] != 0) {
		FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_BOUNCE);
	}
	else {
		FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_BOUNCE);
	}
}

void CatchPickupBall(playerBase* const player) {
	ballBase* const ballInPlayer = playerHitBallInflate[FLAG_TEST(player->playerFlags, PLAYER_SECOND)];

	if (ballInPlayer != NULL)
	{
		if (!FLAG_TEST(ballInPlayer->ballFlags, BALL_NEUTRAL) && !PlayerFacingBall(player, ballInPlayer)) {
			player->playerTimer[PLAYER_CATCH_TIMER] = 0; //also signals ball to bouce
		}

		//catching the ball
		if (player->playerTimer[PLAYER_CATCH_TIMER] != 0)
		{
			//zero out charge if its your own ball
			if (FLAG_TEST(player->playerFlags, PLAYER_SECOND) == FLAG_TEST(ballInPlayer->ballFlags, BALL_ON_PLAYER2)) {
				FLAG_ZERO(ballInPlayer->ballFlags, BALL_CHARGED);
			}

			//let the player move again
			player->playerTimer[PLAYER_CATCH_TIMER] = 0;
			player->playerTimer[PLAYER_STUN_TIMER] = 0;

			//get momentem from ball, after slowing ball down (but don't if the ball is nutral)
			if (!FLAG_TEST(ballInPlayer->ballFlags, BALL_NEUTRAL)) {
				CopySpeedHalf(ballInPlayer->ballPhysics.allSpeeds, ballInPlayer->ballPhysics.allSpeeds);
				AddSpeeds(ballInPlayer->ballPhysics.allSpeeds, player->playerPhysics.allSpeeds);
			}

			//give ball to player
			BallGiveToPlayer(player->playerFlags, ballInPlayer);

			//sounds
			if (FLAG_TEST(ballInPlayer->ballFlags, BALL_CHARGED)) {
				PlaySoundEffect(SOUND_EFFECT_TEST); //changed parry
			}
			else {
				PlaySoundEffect(SOUND_EFFECT_SQEEKIN); //normal catch
			}

			//give player steal protecthing
			player->playerTimer[PLAYER_STEAL_PROTECTION_TIMER] = PLAYER_DODGE_PROTECTION_TIME;
		}

	}
}

void TrowBall(playerBase* const player) {

	if (player->playerTimer[PLAYER_CHARGE_TROW_TIMER] > 0
		&& (!FLAG_TEST(gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))], PAD_ACTION)
			|| player->playerTimer[PLAYER_CHARGE_TROW_TIMER] == 1)) {

		//sounds
		PlaySoundEffect(SOUND_EFFECT_BOOM);
		StopSoundEffect(SOUND_EFFECT_REVERSEDHIT);

		//player gavity back
		FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY); 

		//set ball up
		ballBase* tmpBall = CheckPlayersBall(player->playerFlags);

		//error check
		if (tmpBall == NULL) {
#ifdef NDEBUG
			return;
#else
			printf("TrowBall Null ptr");
			assert(false);
#endif
		}

		//move ball to player
		SetBox(&tmpBall->ballPhysics.postionWorldSpace,
			player->playerPhysics.postionWorldSpace.topLeft.x,
			player->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(PLAYER_TROW_HIGHT_OFFSET));

		//give ball player momentem
		CopySpeed(player->playerPhysics.allSpeeds, tmpBall->ballPhysics.allSpeeds);
		//if player is on ground remove the effects of gravity
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) {
			tmpBall->ballPhysics.allSpeeds[SPEED_DOWN_INDEX] = 0;
		}

		//calc trust speed
		uint8_t trustSpeed = 1 + ((PLAYER_CHARGE_MAX_TIME - player->playerTimer[PLAYER_CHARGE_TROW_TIMER]) >> PLAYER_CHARGE_BITSHIFTER);
		//for parry
		if (FLAG_TEST(tmpBall->ballFlags, BALL_CHARGED)) {
			trustSpeed = MAX_SPEED;
		}
		//limit speed if at 45 degreese
		if (FLAG_TEST(player->playerFlags, PLAYER_TROW_H) && FLAG_TEST(player->playerFlags, PLAYER_TROW_V)) {
			trustSpeed = trustSpeed[TRUST_ANGLE];
		}
		//calc throw speed

		uint8_t trustSpeedTrow = trustSpeed * BASE_TROW_MUTI;
		//if overflow set to max
		if (trustSpeedTrow <= trustSpeed) {
			trustSpeedTrow = MAX_SPEED;
		}

		//put ball's new thrust speed and rocket speed
		if (FLAG_TEST(player->playerFlags, PLAYER_TROW_V)) {
			//trowing down
			if (!FLAG_TEST(player->playerFlags, PLAYER_TROW_UP)) {
				//rocket
				tmpBall->thrust[SPEED_DOWN_INDEX] = trustSpeed;
				//trow
				AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_DOWN_INDEX], trustSpeedTrow);
			}else{ 
				//trowing up
				//rocket trow only works for up at angles, else pure up gets no rocket
				if (FLAG_TEST(player->playerFlags, PLAYER_TROW_H)) {
					//rocket
					tmpBall->thrust[SPEED_UP_INDEX] = trustSpeed;
					//trow
					AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_UP_INDEX], trustSpeedTrow);
				}
				else { //up only with no angle
					//speed for trowing only up, its boosted over normal
					AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_UP_INDEX], trustSpeed * BASE_TROW_MUTI_PURE_UP);
				}
			}
		}

		if (FLAG_TEST(player->playerFlags, PLAYER_TROW_H)) {
			//trowing right
			if (!FLAG_TEST(player->playerFlags, PLAYER_TROW_LEFT)) {
				//rocket
				tmpBall->thrust[SPEED_RIGHT_INDEX] = trustSpeed;
				//trow
				AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX], trustSpeedTrow);
			}
			else {
				//trowing left
				//rocket
				tmpBall->thrust[SPEED_LEFT_INDEX] = trustSpeed;
				//trow
				AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_LEFT_INDEX], trustSpeedTrow);
			}
		}

		//release ball
		FLAG_ZERO(tmpBall->ballFlags, BALL_ON_PLAYER);

		//if ball is moving too slow enable gravity
		if (trustSpeed > PLAYER_CHARGE_MIN_GRAVITY_TRUST
			&& !(FLAG_TEST(player->playerFlags, PLAYER_TROW_V) && FLAG_TEST(player->playerFlags, PLAYER_TROW_UP))) //disable gravity if trowing up 
		{
			FLAG_SET(tmpBall->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
		}
		else {
			FLAG_ZERO(tmpBall->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
		}

		//give player recoil
		SpeedAddInvert(tmpBall->ballPhysics.allSpeeds, player->playerPhysics.allSpeeds, PLAYER_TROW_RECOIL_SHIFT);

		//set timers
		tmpBall->ballTimers[BALL_PARRY_TIMER] = BALL_PARRY_GET_TIME;
		tmpBall->ballTimers[BALL_LOWSPEED_IGNOR] = PLAYER_LOWSPEED_TIME;
		tmpBall->ballTimers[BALL_PLAYER_BOUNCE_IGNOR] = BALL_TIMER_PLAYER_IGNORE_BOUNCE;

		player->playerTimer[PLAYER_CHARGE_TROW_TIMER] = 0;
		player->playerTimer[PLAYER_CANT_ATTACK_TIMER] = TROW_STUN; //used like recovery frames, player cant attack instanty
		player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CHARGE_STUN_RUNOFF; //stun the player a little after a troe
	}
}

void GetTrowDirection(playerBase* const player) {
	if (player->playerTimer[PLAYER_CHARGE_TROW_TIMER] > 0)
	{
		//sound
		PlaySoundEffect(SOUND_EFFECT_REVERSEDHIT);

		//slow player in air
		FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
		FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);

		//get key state for trow direction
		const flags tmpKeysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

		//cancle trow
		if (FLAG_TEST(tmpKeysHeld, PAD_DODGE)) {
			player->playerTimer[PLAYER_CHARGE_TROW_TIMER] = 0;
		}
		
		//if player is holding a dir
		if (FLAG_TEST(tmpKeysHeld, PAD_UP) ||
			FLAG_TEST(tmpKeysHeld, PAD_DOWN) ||
			FLAG_TEST(tmpKeysHeld, PAD_LEFT) ||
			FLAG_TEST(tmpKeysHeld, PAD_RIGHT)) {
			//find what dir the player is holding
			if (FLAG_TEST(tmpKeysHeld, PAD_UP)) {
				FLAG_SET(player->playerFlags, PLAYER_TROW_UP);
				FLAG_SET(player->playerFlags, PLAYER_TROW_V);
			}
			else if (FLAG_TEST(tmpKeysHeld, PAD_DOWN)) {
				FLAG_ZERO(player->playerFlags, PLAYER_TROW_UP);
				FLAG_SET(player->playerFlags, PLAYER_TROW_V);
			}
			else {
				FLAG_ZERO(player->playerFlags, PLAYER_TROW_V);
			}

			if (FLAG_TEST(tmpKeysHeld, PAD_LEFT)) {
				FLAG_SET(player->playerFlags, PLAYER_TROW_LEFT);
				FLAG_SET(player->playerFlags, PLAYER_TROW_H);
			}
			else if (FLAG_TEST(tmpKeysHeld, PAD_RIGHT)) {
				FLAG_ZERO(player->playerFlags, PLAYER_TROW_LEFT);
				FLAG_SET(player->playerFlags, PLAYER_TROW_H);
			}
			else {
				FLAG_ZERO(player->playerFlags, PLAYER_TROW_H);
			}
		}
	}
}

void CatchAndTrowInit(playerBase* const player) {
	const flags keysTap = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	if (FLAG_TEST(keysTap, PAD_ACTION) && player->playerTimer[PLAYER_STUN_TIMER] == 0)
	{
		//pickup ball timer
		if (CheckPlayersBall(player->playerFlags) == NULL)
		{
			if (player->playerTimer[PLAYER_CATCH_TIMER] == 0) {
				player->playerTimer[PLAYER_CATCH_TIMER] = PLAYER_CATCH_TIME;
				player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CATCH_TIME_STUN;
			}
		}//end of pickup ball
		else
			if (player->playerTimer[PLAYER_CANT_ATTACK_TIMER] == 0) //setup charge trow if allowed
			{

				player->playerTimer[PLAYER_CHARGE_TROW_TIMER] = PLAYER_CHARGE_MAX_TIME;
				player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CHARGE_MAX_TIME;

				//defaults based off facing
				FLAG_ZERO(player->playerFlags, PLAYER_TROW_V);
				FLAG_SET(player->playerFlags, PLAYER_TROW_H);
				if (FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT)) {
					FLAG_ZERO(player->playerFlags, PLAYER_TROW_LEFT);
				}
				else {
					FLAG_SET(player->playerFlags, PLAYER_TROW_LEFT);
				}

				//sounds
				if (FLAG_TEST(gs.ball.ballFlags, BALL_CHARGED)) {
					PlaySoundEffect(SOUND_EFFECT_CLICKCLOCK);
				}
				else {
					PlaySoundEffect(SOUND_EFFECT_CLICK);
				}
			}
	}
}

void PlayerDodge(playerBase* const player) {
	const flags keysTap = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	if (FLAG_TEST(keysTap, PAD_DODGE) 
		&& player->playerTimer[PLAYER_STUN_TIMER] == 0
		&& player->playerTimer[PLAYER_DODGE_TIMER_COOLDOWN] == 0) {
		//set player timers
		player->playerTimer[PLAYER_INVISIBILITY_TIMER] = PLAYER_DODGE_TIME;
		player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_DODGE_TIME;
		player->playerTimer[PLAYER_SOLID_TIMER] = PLAYER_DODGE_TIME;
		player->playerTimer[PLAYER_DODGE_TIMER] = PLAYER_DODGE_TIME;
		player->playerTimer[PLAYER_DODGE_TIMER_COOLDOWN] = PLAYER_DODGE_COOLDOWN_TIME;
		//get a small speed step
		uint8_t directionMoving = DIR_NON;
		//find out what direction your moving
		if (player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] < player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]) {
			directionMoving = DIR_RIGHT;
		}
		else if (player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] > 0) {
			directionMoving = DIR_LEFT;
		}
		//if moving up
		if (player->playerPhysics.allSpeeds[SPEED_UP_INDEX] != 0) {
			if (directionMoving == DIR_NON) {
				AddUint8Capped(&player->playerPhysics.allSpeeds[SPEED_UP_INDEX], PLAYER_DODGE_SPEED_BOOST);
			}
			else { //if moving at up and at an angle
				AddUint8Capped(&player->playerPhysics.allSpeeds[SPEED_UP_INDEX], PLAYER_DODGE_SPEED_BOOST_ANGLE);
				if (directionMoving == DIR_RIGHT) {
					AddUint8Capped(&player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX], PLAYER_DODGE_SPEED_BOOST_ANGLE);
				}
				else if (directionMoving == DIR_LEFT) {
					AddUint8Capped(&player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX], PLAYER_DODGE_SPEED_BOOST_ANGLE);
				}
			}
		}
		else { //not moving at an angle, only left and right
			if (directionMoving == DIR_RIGHT) {
				AddUint8Capped(&player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX], PLAYER_DODGE_SPEED_BOOST);
			}
			else if (directionMoving == DIR_LEFT) {
				AddUint8Capped(&player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX], PLAYER_DODGE_SPEED_BOOST);
			}
		}
	}

}

void PlayerJump(playerBase* const player) {
	//het keys states
	flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	flags keysTap = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	bool makeJumpSound = false;

	//particals
	const int16_t playerX = player->playerPhysics.postionWorldSpace.topLeft.x;
	const int16_t playerY = player->playerPhysics.postionWorldSpace.topLeft.y;
	const int16_t playerFeetX = playerX - TO_FIXPOINT(SPRITE_WIDTH_HALF);
	const int16_t playerFeetY = playerY + TO_FIXPOINT(SPRITE_HEIGHT_HALF);
	const int16_t  playerFeetXRngOffset = TO_FIXPOINT(RngMasked8(RNG_MASK_63));
	const int8_t rngVelocityY = -(int8_t)RngMasked8(RNG_MASK_7);
	const uint8_t rngTimer = 14 + RngMasked8(RNG_MASK_31);

	//set if the player can wall jump when not on wall
	if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL) && !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) {
		player->playerTimer[PLAYER_WALLJUMP_TIMER] = PLAYER_WALL_JUMP_TIMER;

		if (FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT)) {
			FLAG_SET(player->playerFlags, PLAYER_TROW_LEFT);
		}
		else {
			FLAG_ZERO(player->playerFlags, PLAYER_TROW_LEFT);
		}
	}

	//stun
	if (player->playerTimer[PLAYER_STUN_TIMER] != 0) {
		keysHeld = 0;
		keysTap = 0;
	}

	//reff speed
	speed* const speeds = player->playerPhysics.allSpeeds;
	
	//jump segment
	if (FLAG_TEST(keysHeld, PAD_JUMP)) {
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) {
			if (FLAG_TEST(player->playerFlags, PLAYER_DUCKING)) { //duck jump
				speeds[SPEED_DOWN_INDEX] = 0;
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_DUCK_JUMP);
			}
			else { //normal jump
				speeds[SPEED_DOWN_INDEX] = 0;
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_JUMP);
			}
			ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, rngVelocityY, 0, 0, SPRITE_INDEX_BIGSMOKE, rngTimer, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
			makeJumpSound = true;
		}
		else if (FLAG_TEST(keysTap, PAD_JUMP))
		{
			if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL) || player->playerTimer[PLAYER_WALLJUMP_TIMER] > 0) { //wall jumping
				
				//hspeed for jump
				speeds[SPEED_DOWN_INDEX] = 0;
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_WALL_JUMPV);

				//set jump
				if (!FLAG_TEST(player->playerFlags, PLAYER_TROW_LEFT)) {
					AddUint8Capped(&speeds[SPEED_RIGHT_INDEX], PLAYER_WALL_JUMPH);
				}
				else {
					AddUint8Capped(&speeds[SPEED_LEFT_INDEX], PLAYER_WALL_JUMPH);
				}

				player->playerTimer[PLAYER_WALLJUMP_TIMER] = 0;
				ParticleAdd(playerFeetX, playerFeetY, 0, rngVelocityY, 0, 0, SPRITE_INDEX_BIGSMOKE, rngTimer, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
				makeJumpSound = true;
			}
			else if (FLAG_TEST(player->playerFlags, PLAYER_HAS_DOUBLE_JUMP)){ //double jumping

				ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, rngVelocityY, 0, 0, SPRITE_INDEX_SMALLSMOKE, rngTimer, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
				makeJumpSound = true;

				FLAG_ZERO(player->playerFlags, PLAYER_HAS_DOUBLE_JUMP);
				//reset gravity
				speeds[SPEED_DOWN_INDEX] = 0;
				//jump
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_DOUBLE_JUMP);
				//for a double jump you get alteast a normal jump
				if (speeds[SPEED_UP_INDEX] < PLAYER_JUMP) {
					speeds[SPEED_UP_INDEX] = PLAYER_JUMP;
				}
			}
		} //end of jump tap
	}//end of jump held

	//sounds
	if (makeJumpSound) {
		PlaySoundEffect(SOUND_EFFECT_SQEEKSLOW);
	}

	//double jump reset
	if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND) || FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {
		FLAG_SET(player->playerFlags, PLAYER_HAS_DOUBLE_JUMP);
	}
}

void PlayerFall(playerBase* const player) {
	//if user presses down while falling this will speed them up
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	speed* const speeds = player->playerPhysics.allSpeeds;

	if (FLAG_TEST(keysHeld, PAD_DOWN) 
		&& !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)
		&& speeds[SPEED_DOWN_INDEX] > speeds[SPEED_UP_INDEX]
		&& !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_WALL)) {
		AddUint8Capped(&speeds[SPEED_UP_INDEX], 0);
		AddUint8Capped(&speeds[SPEED_DOWN_INDEX], PLAYER_FALL_BOOST);
	}
}

//also does wall climbing
void PlayerHspeedMovement(playerBase* const player) {
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	speed* const speeds = player->playerPhysics.allSpeeds;
	int16_t hspeed = 0;

	if (player->playerTimer[PLAYER_STUN_TIMER] == 0)
	{
		if (FLAG_TEST(keysHeld, PAD_LEFT)) {
			hspeed = -1;
			FLAG_ZERO(player->playerFlags, PLAYER_FACING_RIGHT);
		}
		if (FLAG_TEST(keysHeld, PAD_RIGHT)) {
			hspeed = 1;
			FLAG_SET(player->playerFlags, PLAYER_FACING_RIGHT);
		}
	}

	//apply hspeed
	if (hspeed != 0
		&& !FLAG_TEST(player->playerFlags, PLAYER_DUCKING))
	{
		//find what speed to use
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) {
			//on ground
			if (FLAG_TEST(keysHeld, PAD_RUN)) {
				//running
				hspeed *= PLAYER_ACC_GROUND_RUNNING;
			}
			else { //not running
				hspeed *= PLAYER_ACC_GROUND;
			}
		}
		else { //in air
			hspeed *= PLAYER_ACC_AIR;
		}
		//set the player up to move
		if (hspeed < 0) {
			AddUint8Capped(&speeds[SPEED_LEFT_INDEX], -(uint8_t)hspeed);
		}
		else {
			AddUint8Capped(&speeds[SPEED_RIGHT_INDEX], (uint8_t)hspeed);
		}
		//use wall to slow/climb decent
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)
			&& !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) 
		{
			//slide down wall slower
			FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
			ApplyFriction(player->playerPhysics.allSpeeds, BRAKES_FRICTION); //eve more friction to make climbing feel better wen grabbing with a lot of downward speed
			FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);

			//climb up and down
			if (FLAG_TEST(keysHeld, PAD_UP)) {
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_ACC_WALL_CLIMB);
			}
			if (FLAG_TEST(keysHeld, PAD_DOWN)) {
				AddUint8Capped(&speeds[SPEED_DOWN_INDEX], PLAYER_ACC_WALL_CLIMB);
			}

		}//end of wall climb
		else 
		{
			FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
		}
	}
	//make player fall if not wall climbing
	if (!FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {
		FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
	}

}

void PlayerDucking(playerBase* const player) {
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	//ducking
	if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)
		&& FLAG_TEST(keysHeld, PAD_DOWN)
		&& !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)
		&& player->playerTimer[PLAYER_STUN_TIMER] == 0)
	{
		FLAG_SET(player->playerFlags, PLAYER_DUCKING);
		FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
	}
	else {
		FLAG_ZERO(player->playerFlags, PLAYER_DUCKING);
		FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
	}
}

void PlayerSpawning(playerBase* const player) {
	if (player->playerTimer[PLAYER_SPAWN_TIMER]) {
		//make sure the player stays these things when spawning
		player->playerTimer[PLAYER_CANT_ATTACK_TIMER] = 2;
		player->playerTimer[PLAYER_INVISIBILITY_TIMER] = 2;
		
		ballBase* playerBall = CheckPlayersBall(player->playerFlags);
		if (playerBall != NULL) { //remove charge if your hit
			FLAG_ZERO(playerBall->ballFlags, BALL_CHARGED);
		}
	}
}

//AI
void PlayerMoveToAI(const playerBase* const player, uint16_t goalX, uint16_t goalY, bool allowClimb) {

	//this func will try and go to an x/y on the map

	if (player == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("PlayerMoveToAI Null ptr");
		assert(false);
#endif
	}

	//shotcuts to me
	const uint8_t			myPlayerIndex = FLAG_TEST(player->playerFlags, PLAYER_SECOND);
	const speed* const		playerSpeeds = player->playerPhysics.allSpeeds;
	const uint16_t			playerCenterX = player->playerPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	const uint16_t			playerCenterY = player->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(SPRITE_HEIGHT_FORTH);
	//const flags				playerFlags = player->playerFlags;
	const flags				playerPhysicsFlags = player->playerPhysics.physicsFlags;
	//const timer* const		playerTimers = player->playerTimer;
	//shotcuts other player
	const playerBase* const otherPlayer = &gs.players[!myPlayerIndex];
	//const speed* const		otherPlayerspeeds = otherPlayer->playerPhysics.allSpeeds;
	//const uint16_t			otherPlayerCenterX = otherPlayer->playerPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	//const uint16_t			otherPlayerCenterY = otherPlayer->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(SPRITE_HEIGHT_FORTH);
	//const flags				otherPlayerFlags = otherPlayer->playerFlags;
	const flags				otherplayerPhysicsFlags = otherPlayer->playerPhysics.physicsFlags;
	//const timer* const		otherPlayerTimers = otherPlayer->playerTimer;
	//const int32_t			distOtherPlayer = DistancePart(playerCenterX, playerCenterY, otherPlayerCenterX, otherPlayerCenterY);
	//playerBase* const		HitOtherPlayer = playerHitPlayer[myPlayerIndex];
	//shortcuts to ball
	//const uint16_t			ballX = gs.ball.ballPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(SPRITE_WIDTH_FORTH);;
	//const uint16_t			ballY = gs.ball.ballPhysics.postionWorldSpace.topLeft.y - TO_FIXPOINT(SPRITE_HEIGHT_FORTH);;
	//const flags				ballFlags = gs.ball.ballFlags;
	//const int32_t			distBall = DistancePart(playerCenterX, playerCenterY, ballX, ballY);
	//const ballBase* const	ballInPlayer = playerHitBallInflate[myPlayerIndex];
	//shotcuts to IO
	flags* keysHeld = &gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, myPlayerIndex)];
	flags* keysTap = &gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, myPlayerIndex)];
	//maths
	int32_t goalDist = DistancePart(playerCenterX, playerCenterY, goalX, goalY);


	//running
	if (goalDist > TO_FIXPOINT(POW2(AI_DISTANCE_RUN))) {
		FLAG_SET(*keysHeld, PAD_RUN);
	}

	//to get around the plat for MAP_DEBUG
	if (gs.mapIndex == MAP_DEBUG) {
		//check if we are not on the same platfrom
		if ((goalY < TO_FIXPOINT(BASE_RES_HEIGHT_HALF)) != (playerCenterY < TO_FIXPOINT(BASE_RES_HEIGHT_HALF))) {
			goalX = AI_MAP_DEBUG_PLATX_LEFT; // goalX < TO_FIXPOINT(BASE_RES_HEIGHT_HALF) ? AI_MAP_DEBUG_PLATX_LEFT : AI_MAP_DEBUG_PLATX_RIGHT;
			goalY = AI_MAP_DEBUG_PLATY;
		}
	}

	//move left / right when on ground
	if (playerCenterX != goalX) {
		if (playerCenterX < goalX) {
			FLAG_SET(*keysHeld, PAD_RIGHT);
		}
		else {
			FLAG_SET(*keysHeld, PAD_LEFT);
		}
		//climb up if stuck on wall climb
		if (FLAG_TEST(playerPhysicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {
			FLAG_SET(*keysHeld, PAD_UP);
			//jump to get climb started
			if (FLAG_TEST(playerPhysicsFlags, PHYSICS_ONGROUND)) {
				FLAG_SET(*keysTap, PAD_JUMP);
			}
		}
	}

	//jump to player
	if (playerCenterY > goalY) {
		if (FLAG_TEST(playerPhysicsFlags, PHYSICS_ONGROUND)) {
			//duck jump
			FLAG_SET(*keysHeld, PAD_DOWN);
			FLAG_SET(*keysHeld, PAD_JUMP);
		}
		else if (playerSpeeds[SPEED_DOWN_INDEX] > playerSpeeds[SPEED_UP_INDEX] && goalY > TO_FIXPOINT(BASE_RES_HEIGHT_HALF)) {
			//double jump if going down
			FLAG_SET(*keysTap, PAD_JUMP);
			FLAG_SET(*keysHeld, PAD_JUMP);
		}
	}//end of jump

	//wall climb
	if ((allowClimb && FLAG_TEST(otherplayerPhysicsFlags, PHYSICS_IN_HORIZONTAL_WALL))) {

		//hold the wall
		if (goalX > TO_FIXPOINT(BASE_RES_WIDTH_HALF)) {
			FLAG_SET(*keysHeld, PAD_RIGHT);
		}
		else {
			FLAG_SET(*keysHeld, PAD_LEFT);
		}
		//jump onto wall
		if (!FLAG_TEST(playerPhysicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {
			//hop onto wall
			FLAG_SET(*keysHeld, PAD_JUMP);
		}
		else {
			//move up or down to it
			if (playerCenterY < goalY) {
				FLAG_SET(*keysHeld, PAD_DOWN);
			}
			else {
				FLAG_SET(*keysHeld, PAD_UP);
			}
		}
	}//end of climb
}

void PlayerAI(playerBase* const player) {
	
	if (player == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("PlayerAI Null ptr");
		assert(false);
#endif
	}

	//shotcuts to me
	const uint8_t			myPlayerIndex		= FLAG_TEST(player->playerFlags, PLAYER_SECOND);
	//const speed * const		playerSpeeds		= player->playerPhysics.allSpeeds;
	const uint16_t			playerCenterX		= player->playerPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	const uint16_t			playerCenterY		= player->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(SPRITE_HEIGHT_FORTH);
	const flags				playerFlags			= player->playerFlags;
	//const flags				playerPhysicsFlags	= player->playerPhysics.physicsFlags;
	timer* const			playerTimers		= player->playerTimer;
	//shotcuts other player
	const playerBase* const otherPlayer				= &gs.players[!myPlayerIndex];
	//const speed* const		otherPlayerspeeds		= otherPlayer->playerPhysics.allSpeeds;
	const uint16_t			otherPlayerCenterX		= otherPlayer->playerPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	const uint16_t			otherPlayerCenterY		= otherPlayer->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(SPRITE_HEIGHT_FORTH);
	const flags				otherPlayerFlags		= otherPlayer->playerFlags;
	//const flags				otherplayerPhysicsFlags = otherPlayer->playerPhysics.physicsFlags;
	//const timer* const		otherPlayerTimers		= otherPlayer->playerTimer;
	//const int32_t			distOtherPlayer			= DistancePart(playerCenterX, playerCenterY, otherPlayerCenterX, otherPlayerCenterY);
	playerBase* const		HitOtherPlayer			= playerHitPlayer[myPlayerIndex];
	//shortcuts to ball
	const uint16_t			ballX					= gs.ball.ballPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(SPRITE_WIDTH_FORTH);;
	const uint16_t			ballY					= gs.ball.ballPhysics.postionWorldSpace.topLeft.y - TO_FIXPOINT(SPRITE_HEIGHT_FORTH);;
	const flags				ballFlags				= gs.ball.ballFlags;
	const int32_t			distBall				= DistancePart(playerCenterX, playerCenterY, ballX, ballY);
	const ballBase* const	ballInPlayer			= playerHitBallInflate[myPlayerIndex];
	const timer* const		balltimers				= gs.ball.ballTimers;
	//shotcuts to IO
	flags*					keysHeld				= &gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, myPlayerIndex)];
	flags*					keysTap					= &gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, myPlayerIndex)];


	//set AI settings depending on gloable settings
	uint8_t missRate = 0;
	uint8_t trowTimerMask = 0;
	
	if (AI_SET_EASY == gs.settingsAi[myPlayerIndex]) {
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
		missRate = AI_MISS_RATE_MEDIUM;
		trowTimerMask = AI_TROW_TIMER_MASK_MEDIUM;
	}
	else if (AI_SET_HARD == gs.settingsAi[myPlayerIndex]) {
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
		missRate = AI_MISS_RATE_HARD;
		trowTimerMask = AI_MISS_RATE_HARD;
	}
	else if (AI_SET_MEDIUM == gs.settingsAi[myPlayerIndex]) {
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
		missRate = AI_MISS_RATE_MEDIUM;
		trowTimerMask = AI_TROW_TIMER_MASK_MEDIUM;
	}
	else if (AI_SET_FETCH == gs.settingsAi[myPlayerIndex]){
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_SET(player->AI, AI_FETCH);
	}
	else {
		FLAG_ZERO(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
	}


	//main ai start
	if (!FLAG_TEST(player->AI, AI_ENABLED)) return;

	//fetch (needs to come first since it has a early return)
	if (FLAG_TEST(player->AI, AI_FETCH)
		&& (FLAG_TEST(ballFlags, BALL_TOO_FAST) 
		|| CheckPlayersBall(otherPlayerFlags) != NULL
		|| balltimers[BALL_LOWSPEED_IGNOR] != 0)) {
		return;
	}

	//dodging
	if (
		FLAG_TEST(ballFlags, BALL_TOO_FAST)
		&& FLAG_TEST(ballFlags, BALL_ON_PLAYER2) != myPlayerIndex
		&& Rng8() < AI_DODGE_RATE
		&& distBall < TO_FIXPOINT(POW2(AI_DISTANCE_DODGE))
		&& !FLAG_TEST(player->AI, AI_FETCH)) {
		FLAG_SET(*keysTap, PAD_DODGE);
	}

	//random key
	if (Rng8() < AI_RNG_KEY && !FLAG_TEST(player->AI, AI_FETCH)) {
		*keysHeld = Rng8();
		*keysTap = Rng8();
	}

	//getting the ball
	if (CheckPlayersBall(playerFlags) == NULL) {
		//if i dont have the ball
		const bool otherPlayerHasBall = FLAG_TEST(ballFlags, BALL_ON_PLAYER);
		const uint16_t goalX = otherPlayerHasBall ? otherPlayerCenterX : ballX;
		const uint16_t goalY = otherPlayerHasBall ? otherPlayerCenterY : ballY;
		const bool onBall = (otherPlayerHasBall && HitOtherPlayer != NULL) || (!otherPlayerHasBall && ballInPlayer != NULL);
		
		//grab for ball if able to
		if (onBall && Rng8() < missRate) {
			FLAG_SET(*keysTap, PAD_ACTION);
		}

		PlayerMoveToAI(player, goalX, goalY, otherPlayerHasBall);
	}
	else{
		//if you have the ball

		//fetch
		if (FLAG_TEST(player->AI, AI_FETCH) && HitOtherPlayer == NULL) {
			//bring ball back to player
			PlayerMoveToAI(player, otherPlayerCenterX, otherPlayerCenterY, true);
		}
		else {

			//if not touching the player do so
			if (HitOtherPlayer == NULL) {
				player->GoalAIMove.x = otherPlayerCenterX;
				player->GoalAIMove.y = otherPlayerCenterY;
				PlayerMoveToAI(player, player->GoalAIMove.x, player->GoalAIMove.y, false);
			}
			else { //if toching the other player attack
				player->GoalAIMove.x = otherPlayerCenterX - SPRITE_WIDTH + RngMasked8(RNG_MASK_63);
			}

			//player attack start
			if (!FLAG_TEST(player->AI, AI_FETCH)) {
				if (playerTimers[PLAYER_AI_ATTACK_TIMER] == 0) {
					//aim the ball
					if (playerCenterX < otherPlayerCenterX) {
						FLAG_SET(*keysTap, PAD_RIGHT);
					}
					else {
						FLAG_SET(*keysTap, PAD_LEFT);
					}
					FLAG_SET(*keysTap, PAD_ACTION);
					FLAG_SET(*keysHeld, PAD_ACTION);
				}

				//player end attack
				if (playerTimers[PLAYER_AI_ATTACK_TIMER] != 0 && playerTimers[PLAYER_CHARGE_TROW_TIMER] != 0) {
					FLAG_SET(*keysTap, PAD_ACTION);
					FLAG_SET(*keysHeld, PAD_ACTION);
				}

				//start attack timer
				if (playerTimers[PLAYER_AI_ATTACK_TIMER] == 0) {
					playerTimers[PLAYER_AI_ATTACK_TIMER] = Rng8() & trowTimerMask;
				}
			}//end of fetch block

		}
	}
	//get ball end
}
//end of AI

void PlayerStep(playerBase* const player) {
	if (player == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("PlayerStep Null ptr");
		assert(false);
#endif
	}

	//AI comes first to simulate key presses
	PlayerAI(player);
	//make sure these values stay
	PlayerSpawning(player);
	//ducking
	PlayerDucking(player);
	//hspeed
	PlayerHspeedMovement(player);
	//jumping
	PlayerJump(player);
	//fall
	PlayerFall(player);
	//dodging
	PlayerDodge(player);
	//catch / trow ball for action button
	CatchAndTrowInit(player);
	//get hold key state while charging
	GetTrowDirection(player);
	//trow ball
	TrowBall(player);
	//check out if ball in player
	CatchPickupBall(player);
	//if your in a player take their ball if the time is right
	PlayerStealBall(player);
	//bounce timer
	PlayerBounce(player);
	//lower mobily with ball
	if (NULL != CheckPlayersBall(player->playerFlags) && (gs.spriteTimer & 1) /*used spritetimer to halven the effect down*/) {
		ApplyFriction(player->playerPhysics.allSpeeds, AIR_FRICTION);
	}
	//test how long you held the ball (for keep away)
	if (CheckPlayersBall(player->playerFlags) != NULL) {
		player->ballHeldCounter++;
	}
	//move player
	PhysicsStep(&player->playerPhysics);
	//progress timers
	DiscernmentAllTimers(player->playerTimer, PLAYER_TIMER_COUNT);
}

void PlayerSteps(void) {

	//loop though players
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		PlayerStep(&gs.players[i]);
	}
	
}
//
//end player step


void DrawPLayerDebug(const playerBase * const player) {
	//blink
	if (player->playerTimer[PLAYER_BLINK_TIMER] != 0 && !(player->playerTimer[PLAYER_BLINK_TIMER] & PLAYER_BLINK_RATE)) {
		return;
	}

	//pick color for player1/2
	if (FLAG_TEST(player->playerFlags, PLAYER_SECOND)) {
		SDL_SetRenderDrawColor(mainRenderer, PLAYER2_RED_DEBUG, PLAYER2_GREEN_DEBUG, PLAYER2_BLUE_DEBUG, 0xFF);
	}
	else {
		SDL_SetRenderDrawColor(mainRenderer, PLAYER1_RED_DEBUG, PLAYER1_GREEN_DEBUG, PLAYER1_BLUE_DEBUG, 0xFF);
	}
	//draw player
	SDL_Rect tmp = BoxToRec(&player->playerPhysics.postionWorldSpace);
	
	if (FLAG_TEST(player->playerFlags, PLAYER_DUCKING)) {
		int offset = tmp.h - (tmp.h >> PLAYER_HEIGHT_DUCKING_SHIFT);
		tmp.h >>= PLAYER_HEIGHT_DUCKING_SHIFT;
		tmp.y += offset;
	}
	if (player->playerTimer[PLAYER_SOLID_TIMER] != 0) {
		SDL_RenderFillRect(mainRenderer, &tmp);
	}
	else {
		SDL_RenderDrawRect(mainRenderer, &tmp);
	}

	//draw facing
	if (player->playerTimer[PLAYER_CHARGE_TROW_TIMER] == 0) {
		SDL_SetRenderDrawColor(mainRenderer, PLAYER_NOSE_COLOR_RED, PLAYER_NOSE_COLOR_GREEN, PLAYER_NOSE_COLOR_BLUE, 0xFF);
	}
	else
	{
		const uint8_t tmpColor = player->playerTimer[PLAYER_CHARGE_TROW_TIMER] << 3;
		SDL_SetRenderDrawColor(mainRenderer, tmpColor, tmpColor, tmpColor, 0xFF);
	}
	const int halfWidth = tmp.w >> 1;
	tmp.w = halfWidth;
	tmp.h >>= 1;
	if (FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT) == OBJ_FACING_RIGHT) {
		tmp.x += halfWidth;
	}
	if (player->playerTimer[PLAYER_CATCH_TIMER] != 0 || player->playerTimer[PLAYER_CHARGE_TROW_TIMER] != 0) {
		SDL_RenderFillRect(mainRenderer, &tmp);
	}
	else {
		SDL_RenderDrawRect(mainRenderer, &tmp);
	}
}

//also handles some sound effects
void DrawPlayer(const playerBase* const player) {
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	const playerBase* otherPlayer = &gs.players[!FLAG_TEST(player->playerFlags, PLAYER_SECOND)];

	const int16_t xOffset = TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	const int16_t yOffset = 0;
	const int16_t playerX = player->playerPhysics.postionWorldSpace.topLeft.x;
	const int16_t playerY = player->playerPhysics.postionWorldSpace.topLeft.y;
	const int16_t playerFeetX = playerX - TO_FIXPOINT(SPRITE_WIDTH_HALF);
	const int16_t playerFeetY = playerY + TO_FIXPOINT(SPRITE_HEIGHT_HALF);
	const int16_t playerFeetXRngOffset = TO_FIXPOINT(RngMasked8(RNG_MASK_63));
	const int8_t parTimer31 = RngMasked8(RNG_MASK_31);
	uint8_t index = SPRITE_INDEX_PLAYERSTAND;
	bool fast = false;
	bool blink = player->playerTimer[PLAYER_BLINK_TIMER] != 0;

	const bool spawnSmokeRate = (gs.spriteTimer & 1) && (gs.spriteTimer & 2) && (gs.spriteTimer & 4);
	const bool playerMovingH = player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] + player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX] != 0;
	
	//ball offsets
	ballBase* const ball = CheckPlayersBall(player->playerFlags);
	int16_t ballOffX = 0;
	int16_t ballOffY = 0;
	uint8_t ballIndex = SPRITE_INDEX_SMALLBALL;
	bool ballBlink = false;
	bool ballOntopOfPlayer = false;

	//- TO_FIXPOINT(SPRITE_WIDTH_FORTH)
	//- TO_FIXPOINT(SPRITE_HEIGHT_FORTH)

	//sound helper
	if (spriteTimerSound != gs.spriteTimer) {
		if (!playersRunningSound) {
			StopSoundEffect(SOUND_EFFECT_QUICKPITTERPATTER);
		}
		if (!playersWalkingSound) {
			StopSoundEffect(SOUND_EFFECT_PITTERPATTER);
		}

		playersRunningSound = false;
		playersWalkingSound = false;
		spriteTimerSound = gs.spriteTimer;
	}

	//if we have the ball
	if (ball != NULL) {
		ballBlink = FLAG_TEST(ball->ballFlags, BALL_CHARGED);
	}

	if (player->playerTimer[PLAYER_BOUNCH_TIMER] != 0) {
		//getting hurt
		index = SPRITE_INDEX_PLAYERDMG;
		if (spawnSmokeRate) {
			ParticleAdd(playerX, playerY, (int8_t)RngMasked8(RNG_MASK_7) - 14, (int8_t)RngMasked8(RNG_MASK_7) - 14, 0, 0, SPRITE_INDEX_BIGSMOKE, RngMasked8(RNG_MASK_63), FLAG_TEST(player->playerFlags, PLAYER_SECOND));
		}
	} else if (player->playerTimer[PLAYER_DODGE_TIMER_COOLDOWN] != 0
		&& player->playerTimer[PLAYER_INVISIBILITY_TIMER] != 0 ) {
		//dodging
		index = SPRITE_INDEX_PLAYERSHEILD;
	}else if (player->playerTimer[PLAYER_CHARGE_TROW_TIMER] != 0) {
		//trowing
		index = SPRITE_INDEX_PLAYERTROW;
	} else if (player->playerTimer[PLAYER_STEAL_PROTECTION_TIMER] != 0
		|| player->playerTimer[PLAYER_CATCH_TIMER] != 0) {
		//catcing
		if (playerHitPlayer[FLAG_TEST(player->playerFlags, PLAYER_SECOND)] != NULL) {
			//stealing
			index = SPRITE_INDEX_PLAYER_STEAL;
		}
		else {
			//normal catch
			index = SPRITE_INDEX_PLAYERCATCH;
		}
	} else if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND) && !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY)) {
		//if player is on ground
		if (FLAG_TEST(player->playerFlags, PLAYER_DUCKING)) {
			//ducking
			index = SPRITE_INDEX_PLAYERDUCK;
			//duck sliding
			if (spawnSmokeRate && playerMovingH) {
				ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, -3, 0, 0, SPRITE_INDEX_BIGSMOKE, 5 + parTimer31, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
			}
		}
		else if (playerMovingH) {
			if (FLAG_TEST(keysHeld, PAD_LEFT) || FLAG_TEST(keysHeld, PAD_RIGHT)) {
				//if they are moving
				if (FLAG_TEST(keysHeld, PAD_RUN)) {
					//running
					index = SPRITE_INDEX_PLAYERRUN;
					if (screen == SCREEN_STATE_GAME) {
						PlaySoundEffect(SOUND_EFFECT_QUICKPITTERPATTER);
						playersRunningSound = true;
					}
				}
				else {
					//walking
					index = SPRITE_INDEX_PLAYERWALK;
					if (screen == SCREEN_STATE_GAME) {
						PlaySoundEffect(SOUND_EFFECT_PITTERPATTER);
						playersWalkingSound = true;
					}
				}
				
				//smoke if changing directions left
				if (spawnSmokeRate)
				{
					if (FLAG_TEST(keysHeld, PAD_LEFT) && player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] < player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]) {
						ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 3 + (RngMasked8(RNG_MASK_3)), -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 10 + parTimer31, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
					}
					else if (FLAG_TEST(keysHeld, PAD_RIGHT) && player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] > player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]) {
						ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, -(3 + (int8_t)RngMasked8(RNG_MASK_3)), -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 10 + parTimer31, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
					}
				}

			}else{
				//sliding
				index = SPRITE_INDEX_PLAYERSTAND;
				//sliding smoke
				if (spawnSmokeRate) {
					ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 2 + parTimer31, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
				}
			}
		}
		else { //standing default
			index = SPRITE_INDEX_PLAYERSTAND;
		}
	}else{
		//not on ground
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {
			//player wall hold
			if (!FLAG_TEST(keysHeld, PAD_UP) & !FLAG_TEST(keysHeld, PAD_DOWN)) {
				//if not holding up or down while on wall
				//aka wall holding
				index = SPRITE_INDEX_PLAYERWALLHOLD;
			} else if (player->playerPhysics.allSpeeds[SPEED_DOWN_INDEX] > player->playerPhysics.allSpeeds[SPEED_UP_INDEX]) {
				//wall climb down
				index = SPRITE_INDEX_PLAYERCLIMBDOWN;
				if (spawnSmokeRate) {
					ParticleAdd(playerFeetX + TO_FIXPOINT(RngMasked8(RNG_MASK_31)), playerFeetY, 0, -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 2 + parTimer31, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
				}
			}else {
				//wall climb up
				index = SPRITE_INDEX_PLAYERCLIMPUP;
			}
		} else if (player->playerPhysics.allSpeeds[SPEED_DOWN_INDEX] > player->playerPhysics.allSpeeds[SPEED_UP_INDEX]) {
			//falling down
			index = SPRITE_INDEX_PLAYERFLAP;
		}else {
			//falling up
			index = SPRITE_INDEX_PLAYERUP;
		}
	}

	//draw winning pose
	if (replayStartTimer != 0 && otherPlayer->deathCount >= PLAYER_MAX_SCORE) {
		index = SPRITE_INDEX_PLAYERHEADBOUNCH;
	}

	//ball under player
	const uint16_t ballX = playerX + ballOffX;
	const uint16_t ballY = playerY + ballOffY;
	if (!ballOntopOfPlayer && ball != NULL) {
		DrawSprite(ballX, ballY, ballIndex, true, false, false, ballBlink, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
	}

	//draw player
	DrawSprite(playerX - xOffset,
		playerY + yOffset,
		index,
		fast,
		FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT),
		false,
		blink,
		FLAG_TEST(player->playerFlags, PLAYER_SECOND)
	);

	//ball ontop of player
	if (ballOntopOfPlayer && ball != NULL) {
		DrawSprite(ballX, ballY, ballIndex, true, false, false, ballBlink, FLAG_TEST(player->playerFlags, PLAYER_SECOND));
	}

}

void DrawPlayers(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		//DrawPLayerDebug(&gs.players[i]);
		DrawPlayer(&gs.players[i]);
	}
}

void DrawPlayersDebug(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		DrawPLayerDebug(&gs.players[i]);
	}
}


//-=-player scorring-=-

//also draws the winning end game text
void DrawScore(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		const uint16_t x = i ? PLAYER1_DRAW_SCORE_X : PLAYER2_DRAW_SCORE_X;
		const uint16_t y = i ? PLAYER1_DRAW_SCORE_Y : PLAYER2_DRAW_SCORE_Y;
		bool blink = false;

		//draw smoke
		if (gs.players[i].playerTimer[PLAYER_SPAWN_TIMER] > 0) {
			if (Rng8() < 25) {
				ParticleAdd(TO_FIXPOINT(x + RngMasked8(RNG_MASK_63) + SPRITE_WIDTH_HALF), TO_FIXPOINT(y + SPRITE_HEIGHT_HALF), 0, 0, 0, -1, SPRITE_INDEX_BIGSMOKE, RngMasked8(RNG_MASK_63), !i);
			}
			blink = gs.spriteTimer & 8;
		}

		//draw text score
		if (!blink) {
			DrawTextNumberAppend(x, y, true, "", gs.players[i].deathCount);
		}

		//test draw keep away score
		//DrawTextNumberAppend(x, BASE_RES_HEIGHT - 32, false, "", gs.players[i].ballHeldCounter / 60);
	}
}

void CheckPlayersScores(void) {
	
	if ( 
		(
			gs.players[PLAYER_ONE].deathCount >= PLAYER_MAX_SCORE 
			|| gs.players[PLAYER_TWO].deathCount >= PLAYER_MAX_SCORE
			|| gs.gameClock == 0
		)
		&& replayStartTimer == 0 //this timer is also used to tell if the game has ended
		&& !disableGameScore
		) {

		PlaySoundEffect(SOUND_EFFECT_AIRHORN);
		replayStartTimer = REPLAY_START_IN; //start replay timer
	}
}

void DrawEndGameWinningText(void) {
	//show winning text

	//blink texy
	if (disableGameScore)
		return;

	//check if you can show it
	if (
		   gs.players[PLAYER_ONE].deathCount >= PLAYER_MAX_SCORE 
		|| gs.players[PLAYER_TWO].deathCount >= PLAYER_MAX_SCORE 
		|| gs.gameClock == 0
		) {

			//tie game
			if (gs.players[PLAYER_ONE].deathCount == gs.players[PLAYER_TWO].deathCount) {

				DrawText(REPLAY_TEXT_X, REPLAY_TEXT_Y, false, END_TEXT_TIE);

			}
			else {

				char* endText = NULL;
				uint8_t whichPlayerWon = 0;
				
				//which player won
				if (gs.players[PLAYER_ONE].deathCount < gs.players[PLAYER_TWO].deathCount) {
					whichPlayerWon = 1;
				}
				else {
					whichPlayerWon = 2;
				}

				//what kind of win
				if (gs.gameClock == 0) { //timeout
					endText = END_TEXT_TIMEOUT_WIN;
				}
				else if (gs.players[PLAYER_ONE].deathCount == 0 || gs.players[PLAYER_TWO].deathCount == 0){ //perfect
					endText = END_TEXT_PERFECT;
				} else { //normal win
					endText = END_TEXT_WIN;
				}

				DrawTextNumberAppend(REPLAY_TEXT_X, REPLAY_TEXT_Y, false, endText, whichPlayerWon);
			}
	}
	
}

void InitGameClock(void) {
	gs.gameClock = PLAYER_GAME_CLOCK_MAX;
}

void TickGameClock(void) {
	if (gs.gameClock > 0 && !disableGameTimer && !disableGameScore) gs.gameClock--;
}

void DrawGameClock(void) {
	if (!disableGameTimer && !disableGameScore) {
		DrawTextNumberAppend(GAME_CLOCK_X, GAME_CLOCK_Y, true, "", gs.gameClock / TARGET_FRAME_RATE);
	}
}

void RestartMatch(void) {
	InitPlayers();
	InitBall();
	InitParticles();
	InitGameClock();
}

void InitAllScores(void) {
	gs.players[PLAYER_ONE].deathCount = 0;
	gs.players[PLAYER_TWO].deathCount = 0;
	InitGameClock();
}