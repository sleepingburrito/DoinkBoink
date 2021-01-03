#pragma once
#include "physics.h"
#include "Graphics.h"

//init
//
void InitBall(void) {
	ballBase retBall;

	ZeroOut((uint8_t*)&retBall, sizeof(ballBase));
	FLAG_SET(retBall.ballPhysics.physicsFlags, PHYSICS_BOUNCE);

	//ball set start
	retBall.ballPhysics.postionWorldSpace = InitBox(
		TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_BALL_X]),
		TO_FIXPOINT(gs.startSpawnMap[MAP_SPAWN_BALL_Y]),
		TO_FIXPOINT(BALL_SPRITE_HEIGTH),
		TO_FIXPOINT(BALL_SPRITE_WIDTH));

	gs.ball = retBall;
}

//tools
//
//find out if the player owns a ball
ballBase* CheckPlayersBall(const flags playerflag) {
	if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER)
		&& (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2) == FLAG_TEST(playerflag, PLAYER_SECOND))) {
		return &gs.ball;
	}
	else {
		return NULL;
	}
}

//gives the ball the player and inits the ball for storage.
void BallGiveToPlayer(const flags playerflag, ballBase* const ballIn) {
	if (ballIn == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("Null ptr");
		assert(false);
#endif
	}

	ballIn->ballFlags = BitCopy(playerflag, PLAYER_SECOND, ballIn->ballFlags, BALL_ON_PLAYER2);

	FLAG_SET(ballIn->ballFlags, BALL_ON_PLAYER);
	FLAG_SET(ballIn->ballFlags, BALL_INSIDE_PLAYER);
	FLAG_ZERO(ballIn->ballFlags, BALL_NEUTRAL);
	FLAG_ZERO(ballIn->ballFlags, BALL_TOO_FAST);

	FLAG_SET(ballIn->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
	FLAG_ZERO(ballIn->ballPhysics.physicsFlags, PHYSICS_IN_WALL);
	FLAG_ZERO(ballIn->ballPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL);
	FLAG_ZERO(ballIn->ballPhysics.physicsFlags, PHYSICS_BRAKES);

	SetBox(&ballIn->ballPhysics.postionWorldSpace, 0, 0);
	ZeroOutSpeed(ballIn->ballPhysics.allSpeeds);
	ZeroOutSpeed(ballIn->thrust);

	ZeroOut(ballIn->ballTimers, sizeof(timer) * BALL_TIMER_COUNT);

	//set parry timeout
	ballIn->ballTimers[BALL_PARRY_TIMER_TIMEOUT] = BALL_PARRY_TIMEOUT;
}
//
//end tools

//step
//

//set too fast flag based off balls speed
void BallTestTooFast(ballBase* const ball) {
	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("null ptr");
		assert(false);
#endif
	}

	//find out of the ball is moving too fast
	if (SpeedCompare(ball->ballPhysics.allSpeeds) >= (int32_t)(PLAYER_BALL_TOO_FAST * PLAYER_BALL_TOO_FAST)) {
		FLAG_SET(ball->ballFlags, BALL_TOO_FAST);
	}
	else {
		FLAG_ZERO(ball->ballFlags, BALL_TOO_FAST);
	}
}

//critera if the ball can get parried
void BallTestCharged(ballBase* const ball) {
	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("null ptr");
		assert(false);
#endif
	}

	//determin if the ball is charged (for parry)
	if (
			ball->ballTimers[BALL_PARRY_TIMER] > 0
			&& !FLAG_TEST(ball->ballFlags, BALL_NEUTRAL)
			//&& !FLAG_TEST(ball->ballPhysics.physicsFlags, PHYSICS_IN_WALL) //maybe add back, i thinkg this is not needed anymore
		) {
		FLAG_SET(ball->ballFlags, BALL_CHARGED);
	}
	else {
		FLAG_ZERO(ball->ballFlags, BALL_CHARGED);
	}
}

//make the ball safe again
void BallTestNeutral(ballBase* const ball) {

	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("null ptr");
		assert(false);
#endif
	}

	//find out if ball in neutral
	if (!FLAG_TEST(ball->ballFlags, BALL_TOO_FAST)
		&& ball->ballTimers[BALL_LOWSPEED_IGNOR] == 0) {
		FLAG_SET(ball->ballFlags, BALL_NEUTRAL);
	}
}

//checks if you should trust and also trusteses
void RocketBall(ballBase* const ball) {
	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("RocketBall null ptr");
		assert(false);
#endif
	}

	if (FLAG_TEST(ball->ballPhysics.physicsFlags, PHYSICS_IN_WALL)) //kill trust if hit wall 
	{
		ZeroOutSpeed(ball->thrust);
		FLAG_ZERO(ball->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
	}
	//apply thrust to ball
	AddSpeeds(ball->thrust, ball->ballPhysics.allSpeeds);
}

//what to do if you hit a player
void BallInsidePlayer(ballBase* const ball) {
	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("null ptr");
		assert(false);
#endif
	}

	playerBase* playerHit = BallHitPlayer;

	//test for ball overlap with player
	if (playerHit != NULL)
	{
		//if the player was the one to set off the ball
		const bool BallPlayerMatch = FLAG_TEST(playerHit->playerFlags, PLAYER_SECOND) == FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER2);
		
		//let the player know if they got hit by ball (if kill shot)
		if (FLAG_TEST(ball->ballFlags, BALL_TOO_FAST)
			&& playerHit->playerTimer[PLAYER_INVISIBILITY_TIMER] == 0
			&& playerHit->playerTimer[PLAYER_CATCH_TIMER] == 0
			&& !FLAG_TEST(ball->ballFlags, BALL_NEUTRAL)
			&& !BallPlayerMatch) {

			//reset player's current timers
			ZeroOut(playerHit->playerTimer, sizeof(timer) * PLAYER_TIMER_COUNT);

			//stuff to set when player got hit by ball
			playerHit->playerTimer[PLAYER_SPAWN_TIMER] = PLAYER_HIT_TIME;
			playerHit->playerTimer[PLAYER_BLINK_TIMER] = PLAYER_HIT_TIME;
			playerHit->playerTimer[PLAYER_BOUNCH_TIMER] = PLAYER_DEAD_BOUNCE_TIME;
			playerHit->deathCount++;

			//give ball momenum to player
			ApplyFriction(ball->ballPhysics.allSpeeds, GROUND_FRICTION);
			CopySpeedHalf(ball->ballPhysics.allSpeeds, playerHit->playerPhysics.allSpeeds);
			//AddSpeeds(ball->ballPhysics.allSpeeds, playerHit->playerPhysics.allSpeeds);
			//ApplyFriction(playerHit->playerPhysics.allSpeeds, GROUND_FRICTION); //slow player down a little

			//zero ball trust
			ZeroOutSpeed(ball->thrust);
			FLAG_ZERO(ball->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);

			//pause game effect
			gs.worldTimers[WOULD_PAUSE_TIMER] = PLAYER_SCORE_PAUSE_GAME;

			//sound
			PlaySoundEffect(SOUND_EFFECT_HITBONG);
		}

		//bounce off player
		if (
			FLAG_TEST(ball->ballFlags, BALL_INSIDE_PLAYER) == false
			&& (playerHit->playerTimer[PLAYER_CATCH_TIMER] == 0)
			//&& (playerHit->playerTimer[PLAYER_INVISIBILITY_TIMER] == 0)
			&& ball->ballTimers[BALL_PLAYER_BOUNCE_IGNOR] == 0
			&& !FLAG_TEST(playerHit->playerFlags, PLAYER_DUCKING)
			)
		{
			speed tmp = 0;
			const bool playerHitInDodge = playerHit->playerTimer[PLAYER_DODGE_TIMER] > 0;

			//sounds
			if (playerHitInDodge) {
				PlaySoundEffect(SOUND_EFFECT_WORBLE);
			}
			else {
				PlaySoundEffect(SOUND_EFFECT_SQEEKOUTFAST);
			}

			//check if you hit the players head
			if (ball->ballPhysics.postionWorldSpace.topLeft.y > playerHit->playerPhysics.postionWorldSpace.topLeft.y)
			{
				//if you dont hit the players head do a normal bounce
				tmp = ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX];
				ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX] = BRAKES_FRICTION[ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX]];
				ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX] = BRAKES_FRICTION[tmp];
				//give player momentem to ball if in dodge
				if (playerHitInDodge) {
					AddSpeeds(playerHit->playerPhysics.allSpeeds, ball->ballPhysics.allSpeeds);
				}
			}
			else {//if you hit the player head "follow" player
				AddUint8Capped(&ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX], playerHit->playerPhysics.allSpeeds[SPEED_LEFT_INDEX]);
				AddUint8Capped(&ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX], playerHit->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]);
			}
			//up and down
			tmp = ball->ballPhysics.allSpeeds[SPEED_UP_INDEX];
			ball->ballPhysics.allSpeeds[SPEED_UP_INDEX] = BRAKES_FRICTION[ball->ballPhysics.allSpeeds[SPEED_DOWN_INDEX]];
			ball->ballPhysics.allSpeeds[SPEED_DOWN_INDEX] = BRAKES_FRICTION[tmp];
			//bounce off players head
			AddUint8Capped(&ball->ballPhysics.allSpeeds[SPEED_UP_INDEX], playerHit->playerPhysics.allSpeeds[SPEED_UP_INDEX] + PLAYER_HEAD_BALL_BOUNCE);

			//remove trust
			ZeroOutSpeed(ball->thrust);
			FLAG_ZERO(ball->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);

			//change ball allegance if the player is dodging as a parry
			if (//check if the player hit is dodging
				playerHitInDodge
				&& !BallPlayerMatch
				) 
			{
				//change whos side the ball is on
				ball->ballFlags = BitCopy(playerHit->playerFlags, PLAYER_SECOND, ball->ballFlags, BALL_ON_PLAYER2);
				//reset the players dodge
				playerHit->playerTimer[PLAYER_STUN_TIMER] = 0;
				playerHit->playerTimer[PLAYER_SOLID_TIMER] = 0;
				playerHit->playerTimer[PLAYER_INVISIBILITY_TIMER] = 0;
			} //end of parry dodge
		}

		//ball bounce regulation
		FLAG_SET(ball->ballFlags, BALL_INSIDE_PLAYER);
		ball->ballTimers[BALL_PLAYER_BOUNCE_IGNOR] = BALL_TIMER_PLAYER_IGNORE_BOUNCE; //dont bonce off player if you did it too recnalty

	}else{
		FLAG_ZERO(ball->ballFlags, BALL_INSIDE_PLAYER);
	}
}

//where most of the action happens
void BallStep(ballBase * const ball){
	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("Null ptr");
		assert(false);
#endif
	}

	//if not on a player
	if (FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER) == false) {

		//run this first before something turns it off
		BallTestCharged(ball);
		//ball vs player
		BallInsidePlayer(ball);
		//make this safe again
		BallTestNeutral(ball);
		//rocket ball
		RocketBall(ball);
		//move the ball
		PhysicsStep(&ball->ballPhysics);
		//run after physics to find true speed
		BallTestTooFast(ball);

	}
	else //else ball is on player
	{
		//set ball on player
		FLAG_SET(ball->ballFlags, BALL_INSIDE_PLAYER);

		//remove ball parry if held for too long
		if (ball->ballTimers[BALL_PARRY_TIMER_TIMEOUT] < 1) {
			FLAG_ZERO(ball->ballFlags, BALL_CHARGED);
		}
	}

	DiscernmentAllTimers(ball->ballTimers, BALL_TIMER_COUNT);
}

void BallsStep(void) {
	BallStep(&gs.ball);

	//make ball bounce sounds off walls
	if (SpeedCompare(gs.ball.ballPhysics.allSpeeds) > TO_FIXPOINT(SEFFECTS_BALL_MIN_SPEED) 
		&& FLAG_TEST(gs.ball.ballPhysics.physicsFlags, PHYSICS_IN_WALL)) {

		PlaySoundEffect(SOUND_EFFECT_BOUNCE1 + ballSounIndexOffset++);
		if (ballSounIndexOffset >= SEFFECTS_BALL_SOUND_COUNT) {
			ballSounIndexOffset = 0;
		}
	}
}
//
//end step


//Draw
//
void DrawBallDebug(const ballBase* const ball) {
	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("Null ptr");
		assert(false);
#endif
	}

	if (FLAG_TEST(ball->ballFlags, BALL_NEUTRAL)) {
		SDL_SetRenderDrawColor(mainRenderer, BALL_NEUTRAL_DEBUG_R, BALL_NEUTRAL_DEBUG_G, BALL_NEUTRAL_DEBUG_B, 0xFF);
	}
	else { //ball is armed and deadly
		if (FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER2)) {
			SDL_SetRenderDrawColor(mainRenderer, PLAYER2_RED_DEBUG, PLAYER2_GREEN_DEBUG, PLAYER2_BLUE_DEBUG, 0xFF);
		}
		else {
			SDL_SetRenderDrawColor(mainRenderer, PLAYER1_RED_DEBUG, PLAYER1_GREEN_DEBUG, PLAYER1_BLUE_DEBUG, 0xFF);
		}
	}

	SDL_Rect tmp = BoxToRec(&ball->ballPhysics.postionWorldSpace);
	//draw on player if player owned
	if (FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER)) {
		tmp.x = REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.x);
		tmp.y = REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.y);
		if (FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER2)) {
			tmp.x = REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.x);
			tmp.y = REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.y);
		}
		tmp.y += tmp.h >> 1;
	}
	//draw box
	if ((FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER) && FLAG_TEST(ball->ballFlags, BALL_CHARGED))
		|| (FLAG_TEST(ball->ballFlags, BALL_TOO_FAST) && !FLAG_TEST(ball->ballFlags, BALL_NEUTRAL))) {
		SDL_RenderFillRect(mainRenderer, &tmp);
	}
	else {
		SDL_RenderDrawRect(mainRenderer, &tmp);
	}

}

void DrawBall(const ballBase* const ball) {

	if (ball == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("Null ptr");
		assert(false);
#endif
	}

	int16_t xOffset = TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	int16_t yOffset = TO_FIXPOINT(SPRITE_HEIGHT_FORTH);
	int16_t ballX = ball->ballPhysics.postionWorldSpace.topLeft.x;
	int16_t ballY = ball->ballPhysics.postionWorldSpace.topLeft.y;
	uint8_t index = SPRITE_INDEX_BALLSTILL;
	bool inverth = false;
	bool invertv = false;
	bool blink = false;

	const bool spawnSmokeRate = (gs.spriteTimer & 1) && (gs.spriteTimer & 2);
	const bool player2Col = FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER2);

	//draw ball on player if they are holding it
	if (FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER)) {
		return;

		//this code is now on the player
		//if (FLAG_TEST(ball->ballFlags, BALL_ON_PLAYER2)) {
		//	ballX = gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.x;
		//	ballY = gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.y;
		//}else { //player 1
		//	ballX = gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.x;
		//	ballY = gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.y;
		//}

		//blink = FLAG_TEST(ball->ballFlags, BALL_CHARGED);
		//yOffset = 0;
		//index = SPRITE_INDEX_SMALLBALL;
	}

	//change ball sprite if its moving fast enought to hurt player
	if (FLAG_TEST(ball->ballFlags, BALL_TOO_FAST) && !FLAG_TEST(ball->ballFlags, BALL_NEUTRAL)) {
		const int16_t hspeed = (int16_t)ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX] + (int16_t)ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX];

		if (hspeed != 0) { //find which has the greater magantude
			//vspeed
			inverth = ball->ballPhysics.allSpeeds[SPEED_LEFT_INDEX] < ball->ballPhysics.allSpeeds[SPEED_RIGHT_INDEX];
			index = SPRITE_INDEX_BALLHRZ;
		}
		else {
			//hspeed
			invertv = ball->ballPhysics.allSpeeds[SPEED_UP_INDEX] > ball->ballPhysics.allSpeeds[SPEED_DOWN_INDEX];
			index = SPRITE_INDEX_BALLVERT;
		}

		//smoke rings
		if (spawnSmokeRate && ball->ballTimers[BALL_PARRY_TIMER] != 0) {
			ParticleAdd(ballX - xOffset, ballY - yOffset, 0, 0, 0, -1, SPRITE_INDEX_SMOKERING, 15, player2Col);
		}
		//sparks
		if (FLAG_TEST(ball->ballPhysics.physicsFlags, PHYSICS_IN_WALL)) {
			for (uint8_t i = RngMasked8(RNG_MASK_15); i > 0; --i) {
				ParticleAdd(ballX - xOffset, ballY - yOffset, -32 + (5 + (int8_t)(RngMasked8(RNG_MASK_63))), -32 + (5 + (int8_t)(RngMasked8(RNG_MASK_63))), 0, 2, SPRITE_INDEX_SPARK, RngMasked8(RNG_MASK_7), player2Col);
			}
		}
	}

	DrawSprite(ballX - xOffset, ballY - yOffset, index, true, inverth, invertv, blink, player2Col);

}

//
//end draw


