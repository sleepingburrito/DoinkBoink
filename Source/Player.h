#ifndef PLAYER_H
#define PLAYER_H

//--player tools--
//
playerBase InitPlayer(const uint8_t playerIndex) {

#ifdef NDEBUG
#else
	if (playerIndex >= PLAYER_COUNT) {
		printf("InitPlayer index ob");
		assert(false);
	}
#endif

	playerBase returnPlayer;
	ZeroOut((uint8_t*)&returnPlayer, sizeof(playerBase));

	//pause players at start
	returnPlayer.playerTimer[PLAYER_STUN_TIMER] = PLAYER_STUN_START;
	returnPlayer.playerTimer[PLAYER_BLINK_TIMER] = PLAYER_STUN_START;
	returnPlayer.playerTimer[PLAYER_SPAWN_TIMER] = PLAYER_STUN_START;
	returnPlayer.playerTimer[PLAYER_INVISIBILITY_TIMER] = PLAYER_STUN_START;

	//set player size and start location
	if (PLAYER_TWO == playerIndex) {//player 2
		
		FLAG_SET(returnPlayer.playerFlags, PLAYER_SECOND);
		
		returnPlayer.playerPhysics.postionWorldSpace = InitBox(
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER2_X]),
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER2_Y]),
			TO_FIXPOINT(PLAYER_HEIGHT),
			TO_FIXPOINT(PLAYER_WIDTH));
	}
	else {//player 1
		
		returnPlayer.playerPhysics.postionWorldSpace = InitBox(
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER1_X]),
			TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_PLAYER1_Y]),
			TO_FIXPOINT(PLAYER_HEIGHT),
			TO_FIXPOINT(PLAYER_WIDTH));
	}

	//make player face the ball
	CopyTestToFlag(&returnPlayer.playerFlags, PLAYER_FACING_RIGHT, TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_BALL_X]) > returnPlayer.playerPhysics.postionWorldSpace.topLeft.x);

	return returnPlayer;
}

void InitPlayers(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		gs.players[i] = InitPlayer(i);
	}
}

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

	//find out what way the ball is moving
	if (ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX] > ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX]) {
		ballGoingRight = true;
	}

	//check if it matches the same way the player is facing
	bool facing = FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT) != ballGoingRight;

	//if the player is ducking it does not count as facing the ball
	if (FLAG_TEST(player->playerFlags, PLAYER_DUCKING)) {
		facing = false;
	}

	return facing;
}
//
//--end player tools--


//--player step--
//

void PlayerStealBall(playerBase* const player) {
	
	//if in other player
	playerBase* const OtherPlayer = playerHitPlayer[FLAG_TEST(player->playerFlags, PLAYER_SECOND)];
	
	if (OtherPlayer != NULL) {
		//get the other player's ball if they have one
		ballBase* const OtherBall = CheckPlayersBall(OtherPlayer->playerFlags);

		//check if we can steal
		if (player->playerTimer[PLAYER_CATCH_TIMER] != 0
			&& 0 == OtherPlayer->playerTimer[PLAYER_DODGE_TIMER]
			&& 0 == OtherPlayer->playerTimer[PLAYER_STEAL_PROTECTION_TIMER]
			&& 0 == OtherPlayer->playerTimer[PLAYER_CHARGE_THROW_TIMER]
			&& OtherBall != NULL) {

			//let the player move again if they where throwing
			player->playerTimer[PLAYER_CATCH_TIMER] = 0;
			player->playerTimer[PLAYER_STUN_TIMER] = 0;

			//give us steal protection
			player->playerTimer[PLAYER_STEAL_PROTECTION_TIMER] = PLAYER_DODGE_PROTECTION_TIME;

			//tell the ball its mine
			BallGiveToPlayer(player->playerFlags, OtherBall);

			//sound
			PlaySoundEffect(SOUND_EFFECT_DULESQEEK);
		}
	}
}

void PlayerBounce(playerBase* const player) {
	//enable bouncing in physics if the player bounce timer is active
	CopyTestToFlag(&player->playerPhysics.physicsFlags, PHYSICS_BOUNCE, player->playerTimer[PLAYER_BOUNCH_TIMER] != 0);
}

void CatchPickupBall(playerBase* const player) {
	//pickup a neutral ball or catch a ball your facing

	ballBase* const ballInPlayer = playerHitBallInflate[FLAG_TEST(player->playerFlags, PLAYER_SECOND)];

	if (ballInPlayer != NULL)
	{
		//if not facing ball or the ball is not neutral than cancel catching
		if (!FLAG_TEST(ballInPlayer->ballFlags, BALL_NEUTRAL) && !PlayerFacingBall(player, ballInPlayer)) {
			player->playerTimer[PLAYER_CATCH_TIMER] = 0;
		}

		//catching the ball
		if (player->playerTimer[PLAYER_CATCH_TIMER] != 0 && !FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER))
		{
			//zero out charge if its your own ball
			if (FLAG_TEST(player->playerFlags, PLAYER_SECOND) == FLAG_TEST(ballInPlayer->ballFlags, BALL_ON_PLAYER2)) {
				FLAG_ZERO(ballInPlayer->ballFlags, BALL_CHARGED);
			}

			//let the player move again
			player->playerTimer[PLAYER_CATCH_TIMER] = 0;
			player->playerTimer[PLAYER_STUN_TIMER] = 0;

			//get momentum from ball, after slowing ball down (but don't if the ball is neutral)
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

			//give player steal protection
			player->playerTimer[PLAYER_STEAL_PROTECTION_TIMER] = PLAYER_DODGE_PROTECTION_TIME;
		}

	}
}

void ThrowBall(playerBase* const player) {

	if (
		player->playerTimer[PLAYER_CHARGE_THROW_TIMER] > 0
		&& (!FLAG_TEST(gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))], PAD_ACTION)
			|| player->playerTimer[PLAYER_CHARGE_THROW_TIMER] == 1)
		) {

		//sounds
		PlaySoundEffect(SOUND_EFFECT_BOOM);
		StopSoundEffect(SOUND_EFFECT_REVERSEDHIT);

		//player gravity back
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
			player->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(PLAYER_THROW_HIGHT_OFFSET));

		//give ball player momentum
		CopySpeed(player->playerPhysics.allSpeeds, tmpBall->ballPhysics.allSpeeds);
		//if player is on ground remove the effects of gravity
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) {
			tmpBall->ballPhysics.allSpeeds[SPEED_DOWN_INDEX] = 0;
		}

		//calc trust speed
		uint8_t trustSpeed = 1 + ((PLAYER_CHARGE_MAX_TIME - player->playerTimer[PLAYER_CHARGE_THROW_TIMER]) >> PLAYER_CHARGE_BITSHIFTER);

		//for parry
		if (FLAG_TEST(tmpBall->ballFlags, BALL_CHARGED)) {
			trustSpeed = MAX_SPEED;
		}

		//limit speed if at 45 degreese
		if (FLAG_TEST(player->playerFlags, PLAYER_THROW_H) && FLAG_TEST(player->playerFlags, PLAYER_THROW_V)) {
			trustSpeed = trustSpeed[TRUST_ANGLE];
		}

		//calc throw speed
		uint8_t trustSpeedTrow = trustSpeed * BASE_THROW_MUTI;

		//if overflow set to max
		if (trustSpeedTrow <= trustSpeed) {
			trustSpeedTrow = MAX_SPEED;
		}

		//put ball's new thrust speed and rocket speed
		//vspeed
		if (FLAG_TEST(player->playerFlags, PLAYER_THROW_V)) {
			//throwing down
			if (!FLAG_TEST(player->playerFlags, PLAYER_THROW_UP)) {
				//rocket
				tmpBall->thrust[SPEED_DOWN_INDEX] = trustSpeed;
				//throw
				AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_DOWN_INDEX], trustSpeedTrow);
			}else{ 
				//throwing up
				//rocket throw only works for up at angles, else pure up gets no rocket
				if (FLAG_TEST(player->playerFlags, PLAYER_THROW_H)) {
					//rocket
					tmpBall->thrust[SPEED_UP_INDEX] = trustSpeed;
					//throw
					AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_UP_INDEX], trustSpeedTrow);
				}
				else { //up only with no angle
					//speed for throwing only up, its boosted over normal
					AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_UP_INDEX], trustSpeed * BASE_THROW_MUTI_PURE_UP);
				}
			}
		}

		//hspeed
		if (FLAG_TEST(player->playerFlags, PLAYER_THROW_H)) {
			//throwing right
			if (!FLAG_TEST(player->playerFlags, PLAYER_THROW_LEFT)) {
				//rocket
				tmpBall->thrust[SPEED_RIGHT_INDEX] = trustSpeed;
				//throw
				AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX], trustSpeedTrow);
			}
			else {
				//throwing left
				//rocket
				tmpBall->thrust[SPEED_LEFT_INDEX] = trustSpeed;
				//throw
				AddUint8Capped(&tmpBall->ballPhysics.allSpeeds[SPEED_LEFT_INDEX], trustSpeedTrow);
			}
		}

		//if ball is moving too slow enable gravity
		CopyTestToFlag(&tmpBall->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY, (trustSpeed > PLAYER_CHARGE_MIN_GRAVITY_TRUST)
			&& !(FLAG_TEST(player->playerFlags, PLAYER_THROW_V) && FLAG_TEST(player->playerFlags, PLAYER_THROW_UP))); //disable gravity if throwing up 

		//give player recoil
		SpeedAddInvert(tmpBall->ballPhysics.allSpeeds, player->playerPhysics.allSpeeds, PLAYER_THROW_RECOIL_SHIFT);

		//set timers
		//ball
		tmpBall->ballTimers[BALL_PARRY_TIMER] = BALL_PARRY_GET_TIME;
		tmpBall->ballTimers[BALL_LOWSPEED_IGNOR] = PLAYER_LOWSPEED_TIME;
		tmpBall->ballTimers[BALL_PLAYER_BOUNCE_IGNOR] = BALL_TIMER_PLAYER_IGNORE_BOUNCE;

		//player
		player->playerTimer[PLAYER_CHARGE_THROW_TIMER] = 0;
		player->playerTimer[PLAYER_CANT_ATTACK_TIMER] = THROW_STUN; //used like recovery frames, player cant attack instantly
		player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CHARGE_STUN_RUNOFF; //stun the player a little after a throw

		//release ball
		FLAG_ZERO(tmpBall->ballFlags, BALL_ON_PLAYER);

		//particals
		if (FLAG_TEST(player->playerFlags, PLAYER_THROW_H)) {
			const uint16_t xCenterPlayer = player->playerPhysics.postionWorldSpace.topLeft.x + TO_FIXPOINT(PLAYER_WIDTH >> 1) - (FLAG_TEST(player->playerFlags, PLAYER_THROW_LEFT) ? TO_FIXPOINT(PLAYER_WIDTH) : 0);
			const uint16_t yCenterPlayer = player->playerPhysics.postionWorldSpace.topLeft.y + TO_FIXPOINT(PLAYER_HEIGHT >> 2);
			for (uint8_t i = trustSpeedTrow >> 4; i != 0; i--) {
				ParticleAdd(xCenterPlayer, yCenterPlayer, 0, 8 - (int8_t)RngMasked8(RNG_MASK_15), 0, 4 - (int8_t)RngMasked8(RNG_MASK_7), SPRITE_INDEX_BIGSMOKE, RngMasked8(RNG_MASK_15));
			}
		}
	
	}//end of if throwing
}

void GetThrowDirection(playerBase* const player) {
	//while the player is throwing get the throw direction

	if (player->playerTimer[PLAYER_CHARGE_THROW_TIMER] > 0)
	{
		//sound
		PlaySoundEffect(SOUND_EFFECT_REVERSEDHIT);

		//slow player in air
		FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
		FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);

		//get key state for throw direction 
		const flags tmpKeysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

		//throw cancel (or if you loose the ball in a throw)
		if (FLAG_TEST(tmpKeysHeld, PAD_DODGE) || CheckPlayersBall(player->playerFlags) == NULL) {
			player->playerTimer[PLAYER_CHARGE_THROW_TIMER] = 0;
			player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CHARGE_STUN_RUNOFF;
		}
		
		//if player is holding any direction
		if (FLAG_TEST(tmpKeysHeld, PAD_UP) ||
			FLAG_TEST(tmpKeysHeld, PAD_DOWN) ||
			FLAG_TEST(tmpKeysHeld, PAD_LEFT) ||
			FLAG_TEST(tmpKeysHeld, PAD_RIGHT)) {

			//find what direction the player is holding
			//vspeed
			if (FLAG_TEST(tmpKeysHeld, PAD_UP)) {
				FLAG_SET(player->playerFlags, PLAYER_THROW_UP);
				FLAG_SET(player->playerFlags, PLAYER_THROW_V);
			}
			else if (FLAG_TEST(tmpKeysHeld, PAD_DOWN)) {
				FLAG_ZERO(player->playerFlags, PLAYER_THROW_UP);
				FLAG_SET(player->playerFlags, PLAYER_THROW_V);
			}
			else {
				FLAG_ZERO(player->playerFlags, PLAYER_THROW_V);
			}

			//hspeed
			if (FLAG_TEST(tmpKeysHeld, PAD_LEFT)) {
				FLAG_SET(player->playerFlags, PLAYER_THROW_LEFT);
				FLAG_SET(player->playerFlags, PLAYER_THROW_H);
			}
			else if (FLAG_TEST(tmpKeysHeld, PAD_RIGHT)) {
				FLAG_ZERO(player->playerFlags, PLAYER_THROW_LEFT);
				FLAG_SET(player->playerFlags, PLAYER_THROW_H);
			}
			else {
				FLAG_ZERO(player->playerFlags, PLAYER_THROW_H);
			}
		} //end if holding any direction

	}//end if in charge
}

void CatchAndTrowInit(playerBase* const player) {
	const flags keysTap = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	//helpers to check if both players are trying to pickup at the same time
	playerBase* const OtherPlayer = &gs.players[!FLAG_TEST(player->playerFlags, PLAYER_SECOND)];
	const flags keysTapOtherPlayer = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, !FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	if (
		FLAG_TEST(keysTap, PAD_ACTION) && player->playerTimer[PLAYER_STUN_TIMER] == 0
		&& !(
			//if other player is on the ball, and is trying to pickup at the same time as you, then both fail to pick up the ball
			FLAG_TEST(keysTapOtherPlayer, PAD_ACTION)
			&& OtherPlayer->playerTimer[PLAYER_STUN_TIMER] == 0
			&& NULL != playerHitBallInflate[!FLAG_TEST(player->playerFlags, PLAYER_SECOND)]
			&& FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL)
			)
		)
	{
		//pickup ball timer
		if (CheckPlayersBall(player->playerFlags) == NULL)
		{
			if (player->playerTimer[PLAYER_CATCH_TIMER] == 0) {
				player->playerTimer[PLAYER_CATCH_TIMER] = PLAYER_CATCH_TIME;
				player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CATCH_TIME_STUN;
			}
		}//end of pickup ball
		else //start a throw
			if (player->playerTimer[PLAYER_CANT_ATTACK_TIMER] == 0) //setup charge throw if allowed
			{
				player->playerTimer[PLAYER_CHARGE_THROW_TIMER] = PLAYER_CHARGE_MAX_TIME;
				player->playerTimer[PLAYER_STUN_TIMER] = PLAYER_CHARGE_MAX_TIME;

				//defaults for throw direction based off facing
				FLAG_ZERO(player->playerFlags, PLAYER_THROW_V);
				FLAG_SET(player->playerFlags, PLAYER_THROW_H);
				CopyTestToFlag(&player->playerFlags, PLAYER_THROW_LEFT, !FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT));

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

		//get a small speed boost
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
		//end of speed boost

	} //end of dodge
}

void PlayerJump(playerBase* const player) {

	//get keys states
	flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	flags keysTap = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_TAP, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	bool makeJumpSound = false;

	//particals helpers
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
	}

	//stun
	if (player->playerTimer[PLAYER_STUN_TIMER] != 0 || 0 != player->playerTimer[PLAYER_JUMP_COOLDOWN_TIMER]) {
		keysHeld ^= keysHeld;
		keysTap ^= keysTap;
	}

	//speed helper
	speed* const speeds = player->playerPhysics.allSpeeds;
	
	//jump
	if (FLAG_TEST(keysHeld, PAD_JUMP)) {
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) {
			//duck jump
			if (FLAG_TEST(player->playerFlags, PLAYER_DUCKING)) { 
				speeds[SPEED_DOWN_INDEX] = 0;
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_DUCK_JUMP);
			}
			else { 
				//normal jump
				speeds[SPEED_DOWN_INDEX] = 0;
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_JUMP);
			}

			//sound and particles
			ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, rngVelocityY, 0, 0, SPRITE_INDEX_BIGSMOKE, rngTimer);
			makeJumpSound = true;
		}
		else if (FLAG_TEST(keysTap, PAD_JUMP))
		{
			//wall jumping
			if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL) || player->playerTimer[PLAYER_WALLJUMP_TIMER] > 0) { 
				
				//hspeed for jump
				speeds[SPEED_DOWN_INDEX] = 0;
				AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_WALL_JUMPV);

				//find out what direction to do the horizontal wall jump
				//wall jump makes assumptions based around the center of the map, that outside walls jump inwards and inside walls jump outwards
				bool jumpDir = player->playerPhysics.postionWorldSpace.topLeft.x < TO_FIXPOINT(BASE_RES_WIDTH >> 1);

				if (player->playerPhysics.postionWorldSpace.bottomRight.x < TO_FIXPOINT(PLAYER_WALL_JUMP_SIDE_OFFSET)
					|| player->playerPhysics.postionWorldSpace.bottomRight.x > TO_FIXPOINT(BASE_RES_WIDTH - PLAYER_WALL_JUMP_SIDE_OFFSET)) {
					jumpDir = !jumpDir;
				}
				
				//vspeed
				if (!jumpDir) {
					AddUint8Capped(&speeds[SPEED_RIGHT_INDEX], PLAYER_WALL_JUMPH);
				}
				else {
					AddUint8Capped(&speeds[SPEED_LEFT_INDEX], PLAYER_WALL_JUMPH);
				}

				player->playerTimer[PLAYER_WALLJUMP_TIMER] = 0;
				ParticleAdd(playerFeetX, playerFeetY, 0, rngVelocityY, 0, 0, SPRITE_INDEX_BIGSMOKE, rngTimer);
				makeJumpSound = true;
			}
			else //double jumping 
				if (FLAG_TEST(player->playerFlags, PLAYER_HAS_DOUBLE_JUMP)){ 

					ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, rngVelocityY, 0, 0, SPRITE_INDEX_SMALLSMOKE, rngTimer);
					makeJumpSound = true;

					FLAG_ZERO(player->playerFlags, PLAYER_HAS_DOUBLE_JUMP);

					//reset gravity
					speeds[SPEED_DOWN_INDEX] = 0;

					//jump
					AddUint8Capped(&speeds[SPEED_UP_INDEX], PLAYER_DOUBLE_JUMP);

					//for a double jump you get alt east a normal jump
					if (speeds[SPEED_UP_INDEX] < PLAYER_JUMP) {
						speeds[SPEED_UP_INDEX] = PLAYER_JUMP;
					}

			}//end of double jump

		} //end of jump tap
	}//end of jump held

	//sounds
	if (makeJumpSound) {
		PlaySoundEffect(SOUND_EFFECT_SQEEKSLOW);
	}

	//if you hit your head on the celing add a cooldown to jump
	if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_WALL)
		&& !(FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL) || FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND))) {
		
		player->playerTimer[PLAYER_JUMP_COOLDOWN_TIMER] = PLAYER_JUMP_COOLDOWN;
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

void PlayerHspeedMovement(playerBase* const player) {
	//note: wall climbing happens here also

	//helpers
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	speed* const speeds = player->playerPhysics.allSpeeds;
	int16_t hspeed = 0;

	//stun check
	if (0 == player->playerTimer[PLAYER_STUN_TIMER])
	{
		const bool flagStart = FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT); //help find out if you switched direcions

		if (!( FLAG_TEST(keysHeld, PAD_LEFT) && FLAG_TEST(keysHeld, PAD_RIGHT) )) //if pressing both, do nothing
		{
			if (FLAG_TEST(keysHeld, PAD_LEFT)) { //left
				hspeed--;
				FLAG_ZERO(player->playerFlags, PLAYER_FACING_RIGHT);

			} else if (FLAG_TEST(keysHeld, PAD_RIGHT)) { //right
				hspeed++;
				FLAG_SET(player->playerFlags, PLAYER_FACING_RIGHT);
			}

			//start turn animation
			if (flagStart != FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT)) {
				player->playerTimer[PLAYER_TURN_TIMER] = PLAYER_TURN_TIME;
			}
		}//end of checking if left and right are being pressed
	}

	//apply hspeed
	if (
		hspeed != 0
		&& !FLAG_TEST(player->playerFlags, PLAYER_DUCKING)
		)
	{
		//find what speed to use
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) { //on ground

			//start off walking and then start running if moving fast enough
			hspeed *= (GetHspeed(speeds) > TO_FIXPOINT(PLAYER_AUTO_RUN_SPEED)) ? PLAYER_ACC_GROUND_RUNNING : PLAYER_ACC_GROUND;

		}
		else { //in air
			hspeed *= PLAYER_ACC_AIR;
		}

		//Apply acceleration to velocity
		if (hspeed < 0) {
			AddUint8Capped(&speeds[SPEED_LEFT_INDEX], -(uint8_t)hspeed);
		}
		else {
			AddUint8Capped(&speeds[SPEED_RIGHT_INDEX], (uint8_t)hspeed);
		}

		//wall movement 
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_WALL)
			&& !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)) 
		{
			//have the player stick to wall and have more friction
			FLAG_SET(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
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
		{//if not on wall go back to air friction
			FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_BRAKES);
		}
	}

	//make player have gravity if not wall climbing
	if (
		!FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL) 
		&& player->playerTimer[PLAYER_WALLJUMP_TIMER] == 0 //give the player some leeway when holding onto a wall to not just fall for the one frame they don't hold onto it
		) {
		FLAG_ZERO(player->playerPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
	}
}

void PlayerDucking(playerBase* const player) {
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];

	if (
		   FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_ONGROUND)
		&& FLAG_TEST(keysHeld, PAD_DOWN)
		&& !FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)
		&& player->playerTimer[PLAYER_STUN_TIMER] == 0
		)
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

		//make sure the player stays in these states when spawning
		player->playerTimer[PLAYER_CANT_ATTACK_TIMER] = 2;
		player->playerTimer[PLAYER_INVISIBILITY_TIMER] = 2;
		
		//remove ball charge if your hit
		ballBase* const playerBall = CheckPlayersBall(player->playerFlags);
		if (playerBall != NULL) { 
			FLAG_ZERO(playerBall->ballFlags, BALL_CHARGED);
		}

	}
}


//AI
//
void PlayerAiSettings(const uint8_t AiSetting, const uint8_t playerId) {
	if (playerId >= PLAYER_COUNT) {
#ifdef NDEBUG
		return;
#else
		printf("PlayerAiSettings playerId OB");
		assert(false);
#endif
	}

	if (AiSetting >= AI_SETTINGS_COUNT) {
#ifdef NDEBUG
		return;
#else
		printf("PlayerAiSettings AiSetting OB");
		assert(false);
#endif
	}

	//dont run ai in these screen states
	if (screen == SCREEN_STATE_INSTANT_REPLAY || screen == SCREEN_STATE_REWIND) {
		return;
	}

	//load ai settings from gs
	gs.settingsAi[playerId] = AiSetting;
	const uint8_t playerNumTmp = playerId + 1;

	switch (gs.settingsAi[playerId])
	{
	case AI_SET_OFF:
		LogTextScreen(TEXT_AI_OFF, playerNumTmp);
		break;

	case AI_SET_EASY:
		LogTextScreen(TEXT_AI_EASY, playerNumTmp);
		break;

	case AI_SET_MEDIUM:
		LogTextScreen(TEXT_AI_MEDIUM, playerNumTmp);
		break;

	case AI_SET_HARD:
		LogTextScreen(TEXT_AI_HARD, playerNumTmp);
		break;

	case AI_SET_FETCH:
		LogTextScreen(TEXT_AI_FETCH, playerNumTmp);
		break;
#ifdef NDEBUG
		
#else
	default:
		LogTextScreen("ERR: AI INDEX OB", playerNumTmp);
#endif

	}
}

void PlayerMoveToAI(const playerBase* const player, uint16_t goalX, uint16_t goalY, bool allowClimb) {

	//this will try and get the the player to the x and y given

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

	//to get around the platform for MAP_DEBUG
	if (gs.mapIndex == MAP_DEBUG) {
		//check if we are not on the same platfrom
		if ((goalY < TO_FIXPOINT(BASE_RES_HEIGHT_HALF + SPRITE_HEIGHT)) != (playerCenterY < TO_FIXPOINT(BASE_RES_HEIGHT_HALF + SPRITE_HEIGHT))) {

			goalX = goalX < TO_FIXPOINT(BASE_RES_HEIGHT_HALF) ? AI_MAP_DEBUG_PLATX_LEFT - TO_FIXPOINT(SPRITE_WIDTH) : AI_MAP_DEBUG_PLATX_RIGHT;
			goalY = AI_MAP_DEBUG_PLATY;

			int16_t distX = (int16_t)playerCenterX - (int16_t)goalX;
			if (distX < 0) distX = -distX;

			//try and jump up onto the platfrom
			if (
				(distX < TO_FIXPOINT(SPRITE_WIDTH))
				&& playerCenterY > goalY
				) {
				FLAG_SET(*keysTap, PAD_JUMP);
			}
		}
	}

	//dont get stuck under the plat on big S
	if (gs.mapIndex == MAP_BIG_S) {

		int16_t distX = (int16_t)playerCenterX - (int16_t)TO_FIXPOINT(BASE_RES_WIDTH_HALF - SPRITE_WIDTH_HALF);
		if (distX < 0) distX = -distX;

		//if the ball is on top hold a dir to avoid the plat
		if (distX < TO_FIXPOINT(SPRITE_WIDTH) && goalY < playerCenterY) {
			FLAG_SET(*keysHeld, PAD_RIGHT);
		}
	}


	//goals are normally in the middle of a sprite so i'm adding a sprite buffer to keep the players from flailing around
	const uint16_t goalLeft = goalX - SPRITE_WIDTH;
	const uint16_t goalRight = goalX + SPRITE_WIDTH;
	const uint16_t goalUp = goalY - SPRITE_HEIGHT;
	const uint16_t goalDown = goalY + SPRITE_HEIGHT;

	//move left / right when on ground
	if (playerCenterX != goalX) {

		if (playerCenterX < goalLeft) {
			FLAG_SET(*keysHeld, PAD_RIGHT);
		}
		else if (playerCenterX > goalRight){
			FLAG_SET(*keysHeld, PAD_LEFT);
		}

		//climb up if stuck on wall climb
		if (FLAG_TEST(playerPhysicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {

			if (goalUp < playerCenterY) {//climb tords goal
				FLAG_SET(*keysHeld, PAD_UP);
			}
			else if (goalDown > playerCenterY){
				FLAG_SET(*keysHeld, PAD_DOWN);
			}

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
			if (playerCenterY < goalDown) {
				FLAG_SET(*keysHeld, PAD_DOWN);
			}
			else if (playerCenterY > goalUp){
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
	uint8_t randomKeyRate = 0;
	uint8_t mistakeTimer = 0;
	
	if (AI_SET_EASY == gs.settingsAi[myPlayerIndex]) {
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
		missRate = AI_MISS_RATE_EASY;
		randomKeyRate = AI_RNG_KEY_EASY;
		mistakeTimer = RNG_MASK_127;
	}
	else if (AI_SET_HARD == gs.settingsAi[myPlayerIndex]) {
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
		missRate = AI_MISS_RATE_HARD;
		randomKeyRate = AI_RNG_KEY_MEDIUM;
		mistakeTimer = RNG_MASK_31;
	}
	else if (AI_SET_MEDIUM == gs.settingsAi[myPlayerIndex]) {
		FLAG_SET(player->AI, AI_ENABLED);
		FLAG_ZERO(player->AI, AI_FETCH);
		missRate = AI_MISS_RATE_MEDIUM;
		randomKeyRate = AI_RNG_KEY_HARD;
		mistakeTimer = RNG_MASK_63;
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

	//fetch (needs to come first since it has an early return)
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
		&& !FLAG_TEST(player->AI, AI_FETCH)
		&& playerTimers[PLAYER_AI_INMISS_TIMER] < 1
		) {

		FLAG_SET(*keysTap, PAD_DODGE);
	}

	//random key
	if (Rng8() < randomKeyRate && !FLAG_TEST(player->AI, AI_FETCH)) {
		*keysHeld = Rng8();
		*keysTap = Rng8();

		//make sure to not hold both dir keys at the same time
		*keysHeld &= ~(1 << (myPlayerIndex ? PAD_LEFT : PAD_RIGHT) );
	}

	//getting the ball
	if (CheckPlayersBall(playerFlags) == NULL) {
		//if i dont have the ball
		const bool otherPlayerHasBall = FLAG_TEST(ballFlags, BALL_ON_PLAYER);
		const uint16_t goalX = otherPlayerHasBall ? otherPlayerCenterX : ballX;
		const uint16_t goalY = otherPlayerHasBall ? otherPlayerCenterY : ballY;
		const bool onBall = (otherPlayerHasBall && HitOtherPlayer != NULL) || (!otherPlayerHasBall && ballInPlayer != NULL);
		
		//grab for ball if able to
		if (onBall && playerTimers[PLAYER_AI_INMISS_TIMER] < 1) {

			if (Rng8() < missRate || FLAG_TEST(player->AI, AI_FETCH)) {
				FLAG_SET(*keysTap, PAD_ACTION);
			}
			else if (FLAG_TEST(ballFlags, BALL_TOO_FAST)){ //Make mistake punishment less when the ball is moving slower
				playerTimers[PLAYER_AI_INMISS_TIMER] = RngMasked8(mistakeTimer);
			}
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
				if (playerTimers[PLAYER_AI_ATTACK_TIMER] != 0 && playerTimers[PLAYER_CHARGE_THROW_TIMER] != 0) {
					FLAG_SET(*keysTap, PAD_ACTION);
					FLAG_SET(*keysHeld, PAD_ACTION);
				}

				//start attack timer
				if (playerTimers[PLAYER_AI_ATTACK_TIMER] == 0) {
					playerTimers[PLAYER_AI_ATTACK_TIMER] = Rng8() & AI_THROW_TIMER_MASK_MEDIUM;
				}
			}//end of fetch block

		}
	}
	//get ball end
}
//end of AI

void PlayerStep(const uint8_t index) {

	playerBase* const player = &gs.players[index & 1];

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

	//catch / throw ball for action button
	CatchAndTrowInit(player);

	//get hold key state while charging
	GetThrowDirection(player);

	//throw ball
	ThrowBall(player);

	//check out if ball in player
	CatchPickupBall(player);

	//if your in a player take their ball if the time is right
	PlayerStealBall(player);

	//bounce timer
	PlayerBounce(player);

	//lower mobily with ball
	if (NULL != CheckPlayersBall(player->playerFlags) && (gs.spriteTimer & 1) /*used spritetimer to halve the effect*/) {
		ApplyFriction(player->playerPhysics.allSpeeds, AIR_FRICTION);
	}

	//move player
	PhysicsStep(&player->playerPhysics);

	//progress timers
	DiscernmentAllTimers(player->playerTimer, PLAYER_TIMER_COUNT);
}

void PlayerSteps(void) {

	//loop though players
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		PlayerStep(i);
	}
	
}
//
//--end player step--


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
	if (player->playerTimer[PLAYER_CHARGE_THROW_TIMER] == 0) {
		SDL_SetRenderDrawColor(mainRenderer, PLAYER_NOSE_COLOR_RED, PLAYER_NOSE_COLOR_GREEN, PLAYER_NOSE_COLOR_BLUE, 0xFF);
	}
	else
	{
		const uint8_t tmpColor = player->playerTimer[PLAYER_CHARGE_THROW_TIMER] << 3;
		SDL_SetRenderDrawColor(mainRenderer, tmpColor, tmpColor, tmpColor, 0xFF);
	}
	const int halfWidth = tmp.w >> 1;
	tmp.w = halfWidth;
	tmp.h >>= 1;
	if (FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT) == OBJ_FACING_RIGHT) {
		tmp.x += halfWidth;
	}

	if (player->playerTimer[PLAYER_CATCH_TIMER] != 0 || player->playerTimer[PLAYER_CHARGE_THROW_TIMER] != 0) {
		SDL_RenderFillRect(mainRenderer, &tmp);
	}
	else {
		SDL_RenderDrawRect(mainRenderer, &tmp);
	}
}

void DrawPlayer(const playerBase* const player) {
	//note: also handles some sound effects

	//helpers
	const flags keysHeld = gs.padIOReadOnly[PADIO_INDEX(PAD_STATE_HELD, FLAG_TEST(player->playerFlags, PLAYER_SECOND))];
	const playerBase* otherPlayer = &gs.players[!FLAG_TEST(player->playerFlags, PLAYER_SECOND)];
	const speed* const speeds = player->playerPhysics.allSpeeds;

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
	const int16_t playerDownBallOffset = (SPRITE_HEIGHT << 1) + SPRITE_HEIGHT_FORTH;

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

	if (player->playerTimer[PLAYER_BOUNCH_TIMER] != 0 && player->playerTimer[PLAYER_SPAWN_TIMER] != 0) {
		//getting hurt
		index = SPRITE_INDEX_PLAYERDMG;
		if (spawnSmokeRate) {
			ParticleAdd(playerX, playerY, (int8_t)RngMasked8(RNG_MASK_7) - 14, (int8_t)RngMasked8(RNG_MASK_7) - 14, 0, 0, SPRITE_INDEX_BIGSMOKE, RngMasked8(RNG_MASK_63));
		}
	}
	else if (player->playerTimer[PLAYER_TURN_TIMER] != 0){
		//turning animation
		index = SPRITE_INDEX_PLAYERTURN;

	} else if (player->playerTimer[PLAYER_DODGE_TIMER_COOLDOWN] != 0
		&& player->playerTimer[PLAYER_INVISIBILITY_TIMER] != 0 ) {
		//dodging
		index = SPRITE_INDEX_PLAYERSHEILD;
	}else if (player->playerTimer[PLAYER_CHARGE_THROW_TIMER] != 0) {
		//throwing
		index = SPRITE_INDEX_PLAYERTHROW;
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
			ballOffY += playerDownBallOffset;
			//duck sliding
			if (spawnSmokeRate && playerMovingH) {
				ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, -3, 0, 0, SPRITE_INDEX_BIGSMOKE, 5 + parTimer31);
			}
		}
		else if (playerMovingH) {
			if (FLAG_TEST(keysHeld, PAD_LEFT) || FLAG_TEST(keysHeld, PAD_RIGHT)) {
				//if they are moving
				if (GetHspeed(speeds) > TO_FIXPOINT(PLAYER_AUTO_RUN_SPEED)) {
					//running
					index = SPRITE_INDEX_PLAYERRUN;
					ballOffY += playerDownBallOffset;
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

				//smoke if changing directions
				if (spawnSmokeRate)
				{
					if (FLAG_TEST(keysHeld, PAD_LEFT) && player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] < player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]) {
						ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 3 + (RngMasked8(RNG_MASK_3)), -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 10 + parTimer31);
					}
					else if (FLAG_TEST(keysHeld, PAD_RIGHT) && player->playerPhysics.allSpeeds[SPEED_LEFT_INDEX] > player->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]) {
						ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, -(3 + (int8_t)RngMasked8(RNG_MASK_3)), -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 10 + parTimer31);
					}
				}

			}else{
				//sliding
				index = SPRITE_INDEX_PLAYERSTAND;
				//sliding smoke
				if (spawnSmokeRate) {
					ParticleAdd(playerFeetX + playerFeetXRngOffset, playerFeetY, 0, -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 2 + parTimer31);
				}
			}
		}
		else { //standing default
			index = SPRITE_INDEX_PLAYERSTAND;
			ballOntopOfPlayer = true;
		}
	}else{
		//not on ground
		if (FLAG_TEST(player->playerPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL)) {
			//player wall hold
			if (player->playerPhysics.allSpeeds[SPEED_DOWN_INDEX] == player->playerPhysics.allSpeeds[SPEED_UP_INDEX]) {
				//if not holding up or down while on wall
				//aka wall holding
				index = SPRITE_INDEX_PLAYERWALLHOLD;
			} else if (player->playerPhysics.allSpeeds[SPEED_DOWN_INDEX] > player->playerPhysics.allSpeeds[SPEED_UP_INDEX]) {
				//wall climb down
				index = SPRITE_INDEX_PLAYERCLIMBDOWN;
				if (spawnSmokeRate) {
					ParticleAdd(playerFeetX + TO_FIXPOINT(RngMasked8(RNG_MASK_31)), playerFeetY, 0, -3, 0, 0, SPRITE_INDEX_SMALLSMOKE, 2 + parTimer31);
				}
			}else {
				//wall climb up
				index = SPRITE_INDEX_PLAYERCLIMPUP;
			}
		} else if (player->playerPhysics.allSpeeds[SPEED_DOWN_INDEX] > player->playerPhysics.allSpeeds[SPEED_UP_INDEX]) {
			//falling down
			index = SPRITE_INDEX_PLAYERFLAP;
		}else {
			//going up
			index = SPRITE_INDEX_PLAYERUP;
		}
	}

	//draw winning pose
	if (replayStartTimer != 0 && otherPlayer->deathCount >= PLAYER_MAX_SCORE) {
		index = SPRITE_INDEX_PLAYERHEADBOUNCH;
	}

	//ball on top of player in these cases
	if (
		SPRITE_INDEX_PLAYERSTAND == index 
		|| SPRITE_INDEX_PLAYERWALK == index
		|| SPRITE_INDEX_PLAYERRUN == index
		|| SPRITE_INDEX_PLAYERWALLHOLD == index
		|| SPRITE_INDEX_PLAYERCLIMBDOWN == index
		|| SPRITE_INDEX_PLAYERCLIMPUP == index
		) {
		ballOntopOfPlayer = true;
		
	}

	//bob ball sprite when walking
	if (((gs.spriteTimer >> SPRITE_SPEED_NORMAL) & 1) && SPRITE_INDEX_PLAYERWALK == index) {
		ballOffY += SPRITE_HEIGHT_HALF;
	}
	if (SPRITE_INDEX_PLAYERSTAND == index || SPRITE_INDEX_PLAYERWALK == index) {
		ballOffY += SPRITE_HEIGHT;
	}

	//offset ball when the player is facing right
	if (!FLAG_TEST(player->playerFlags, PLAYER_FACING_RIGHT)) {
		ballOffX -= SPRITE_WIDTH << 2;
	}

	//blink charged ball
	if (ball != NULL) {
		ballBlink = FLAG_TEST(ball->ballFlags, BALL_CHARGED);
	}

	//ball offsets
	int16_t ballX = (int16_t)playerX + ballOffX;
	int16_t ballY = playerY + ballOffY;

	//out of bounds ball reset to the player's location
	if (ball != NULL) {
		if (ballX <= 0 || ballX >= TO_FIXPOINT(BASE_RES_WIDTH - SPRITE_WIDTH)) {
			ballX = playerX;
		}
		if (ballY <= 0 || ballY >= TO_FIXPOINT(BASE_RES_HEIGHT - SPRITE_HEIGHT)) {
			ballY = playerY;
		}
	}

	//draw ball under player
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
		DrawPlayer(&gs.players[i]);
	}
}

void DrawPlayersDebug(void) {
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		DrawPLayerDebug(&gs.players[i]);
	}
}


//--player scoring--

void DrawScore(void) {
	//also draws the winning end game text
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
		const uint16_t x = i ? PLAYER1_DRAW_SCORE_X : PLAYER2_DRAW_SCORE_X;
		const uint16_t y = i ? PLAYER1_DRAW_SCORE_Y : PLAYER2_DRAW_SCORE_Y;
		bool blink = false;

		//draw smoke
		if (gs.players[i].playerTimer[PLAYER_SPAWN_TIMER] > 0) {
			if (Rng8() < 25) {
				ParticleAdd(TO_FIXPOINT(x + RngMasked8(RNG_MASK_63) + SPRITE_WIDTH_HALF), TO_FIXPOINT(y + SPRITE_HEIGHT_HALF), 0, 0, 0, -1, SPRITE_INDEX_BIGSMOKE, RngMasked8(RNG_MASK_63));
			}
			blink = gs.spriteTimer & 8;
		}

		//draw text score
		if (!blink) {
			DrawTextNumberAppend(x, y, true, "", gs.players[i].deathCount);
		}

	}
}

void CheckPlayersScores(void) {
	
	if ( //check if the game is over
		(
			gs.players[PLAYER_ONE].deathCount >= PLAYER_MAX_SCORE 
			|| gs.players[PLAYER_TWO].deathCount >= PLAYER_MAX_SCORE
			|| gs.gameClock == 0
		)
		&& replayStartTimer == 0 //this timer is also used to tell if the game has ended in other parts of the code
		&& !disableGameScore
		) 
	{

		
#ifdef NDEBUG
#else
		//debug print players scores (helps with AI map testing)
		static int p1Score = 0;
		static int p2Score = 0;
		if (gs.players[PLAYER_ONE].deathCount < gs.players[PLAYER_TWO].deathCount) {
			printf("P1 WON, SCORE OTHER PLAYER: %d ", gs.players[PLAYER_ONE].deathCount);
			p1Score++;
		}
		else {
			printf("P2 WON, SCORE OTHER PLAYER: %d ", gs.players[PLAYER_TWO].deathCount);
			p2Score++;
		}
		printf(" Score Total P1: %d P2: %d \n", p1Score, p2Score);
#endif

		//end and sound
		PlaySoundEffect(SOUND_EFFECT_AIRHORN);
		replayStartTimer = REPLAY_START_IN;
	}
}

void DrawEndGameWinningText(void) {
	//show winning text

	//blink text
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
	for (uint8_t i = 0; i < PLAYER_COUNT; ++i) 
		gs.players[i].deathCount = 0;
	
	InitGameClock();
}

#endif