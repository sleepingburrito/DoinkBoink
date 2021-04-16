#ifndef BALL_H
#define BALL_H

#include "Physics.h"

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

	return NULL;
}

//gives the ball the player and inits the ball for storage
void BallGiveToPlayer(const flags playerflag, ballBase* const ballIn) {
	if (ballIn == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("Null ptr");
		assert(false);
#endif
	}

	//which player has the ball now
	ballIn->ballFlags = BitCopy(playerflag, PLAYER_SECOND, ballIn->ballFlags, BALL_ON_PLAYER2);

	//main ball flags
	FLAG_SET(ballIn->ballFlags, BALL_ON_PLAYER);
	FLAG_SET(ballIn->ballFlags, BALL_INSIDE_PLAYER);
	FLAG_ZERO(ballIn->ballFlags, BALL_NEUTRAL);
	FLAG_ZERO(ballIn->ballFlags, BALL_TOO_FAST);

	FLAG_SET(ballIn->ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
	FLAG_ZERO(ballIn->ballPhysics.physicsFlags, PHYSICS_IN_WALL);
	FLAG_ZERO(ballIn->ballPhysics.physicsFlags, PHYSICS_IN_HORIZONTAL_WALL);
	FLAG_ZERO(ballIn->ballPhysics.physicsFlags, PHYSICS_BRAKES);

	//other zero outs
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
//set too fast flag based off ball's speed
void BallTestTooFast(void) {

	//find out if the ball is moving too fast
	CopyTestToFlag(&gs.ball.ballFlags, BALL_TOO_FAST, SpeedCompare(gs.ball.ballPhysics.allSpeeds) >= (int32_t)POW2(PLAYER_BALL_TOO_FAST));

}

//criteria if the ball can get parried caught, will set ball charged flag
void BallTestCharged(void) {
	//determine if the ball is charged (for parry)
	CopyTestToFlag(&gs.ball.ballFlags, BALL_CHARGED, gs.ball.ballTimers[BALL_PARRY_TIMER] > 0 && !FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL));

}

//make the ball safe again
void BallTestNeutral(void) {

	//set ball back to neutral if moving too slow
	if (!FLAG_TEST(gs.ball.ballFlags, BALL_TOO_FAST) && 0 == gs.ball.ballTimers[BALL_LOWSPEED_IGNOR]) {
		FLAG_SET(gs.ball.ballFlags, BALL_NEUTRAL);
	}
}

//checks if you should thrust and also applies thrust to velocity
void BallStopThrust(void) {
	ZeroOutSpeed(gs.ball.thrust);
	FLAG_ZERO(gs.ball.ballPhysics.physicsFlags, PHYSICS_DISABLE_GRAVITY);
}

void RocketBall(void) {

	if (FLAG_TEST(gs.ball.ballPhysics.physicsFlags, PHYSICS_IN_WALL)) //kill thrust if hit wall 
	{
		BallStopThrust();
	}

	//apply thrust to ball (if there is any)
	AddSpeeds(gs.ball.thrust, gs.ball.ballPhysics.allSpeeds);
}

//what to do if you hit a player
void BallInsidePlayer(void) {

	playerBase* playerHit = BallHitPlayer;

	//test for ball overlap with player
	if (playerHit != NULL)
	{
		//check if we hit the player that threw the ball
		const bool BallPlayerMatch = FLAG_TEST(playerHit->playerFlags, PLAYER_SECOND) == FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2);
		
		//let the player know if they got hit by ball (if a score shot)
		if (FLAG_TEST(gs.ball.ballFlags, BALL_TOO_FAST)
			&& playerHit->playerTimer[PLAYER_INVISIBILITY_TIMER] == 0
			&& playerHit->playerTimer[PLAYER_CATCH_TIMER] == 0
			&& !FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL)
			&& !BallPlayerMatch) {

			//reset player's current timers
			ZeroOut(playerHit->playerTimer, sizeof(timer) * PLAYER_TIMER_COUNT);

			//stuff to set when player got hit by ball
			playerHit->playerTimer[PLAYER_SPAWN_TIMER] = PLAYER_HIT_TIME;
			playerHit->playerTimer[PLAYER_BLINK_TIMER] = PLAYER_HIT_TIME;
			playerHit->playerTimer[PLAYER_BOUNCH_TIMER] = PLAYER_DEAD_BOUNCE_TIME;
			playerHit->playerTimer[PLAYER_STUN_TIMER] = (PLAYER_DEAD_BOUNCE_TIME / 2);
			playerHit->playerTimer[PLAYER_WALLJUMP_TIMER] = 0; //being safe with this one, not sure if its super needed
			playerHit->deathCount++;

			//give ball momentum to player 
			ApplyFriction(gs.ball.ballPhysics.allSpeeds, GROUND_FRICTION);
			CopySpeedHalf(gs.ball.ballPhysics.allSpeeds, playerHit->playerPhysics.allSpeeds);

			//zero ball trust
			BallStopThrust();
			
			//make ball safe
			FLAG_SET(gs.ball.ballFlags, BALL_NEUTRAL);

			//pause game effect
			gs.worldTimers[WOULD_PAUSE_TIMER] = PLAYER_SCORE_PAUSE_GAME;

			//sound
			PlaySoundEffect(SOUND_EFFECT_HITBONG);

			//screen shake
			gs.backgroundShakeRate = BACKGROUND_SHAKE_START_RATE;
		}

		//bounce off player
		if (
			!FLAG_TEST(gs.ball.ballFlags, BALL_INSIDE_PLAYER)
			&& 0 == playerHit->playerTimer[PLAYER_CATCH_TIMER]
			&& 0 == gs.ball.ballTimers[BALL_PLAYER_BOUNCE_IGNOR]
			&& !FLAG_TEST(playerHit->playerFlags, PLAYER_DUCKING)
			)
		{
			speed tmp = 0;
			const bool playerHitInDodge = playerHit->playerTimer[PLAYER_DODGE_TIMER] > 0;

			//sounds
			if (playerHitInDodge) {
				PlaySoundEffect(SOUND_EFFECT_WORBLE); //block hit
			}
			else {
				PlaySoundEffect(SOUND_EFFECT_SQEEKOUTFAST); //normal hit
			}

			//check if you hit the players head
			if (gs.ball.ballPhysics.postionWorldSpace.topLeft.y > playerHit->playerPhysics.postionWorldSpace.topLeft.y)
			{
				//if you dont hit the player's head, do a normal bounce
				tmp = gs.ball.ballPhysics.allSpeeds[SPEED_LEFT_INDEX];
				gs.ball.ballPhysics.allSpeeds[SPEED_LEFT_INDEX] = BRAKES_FRICTION[gs.ball.ballPhysics.allSpeeds[SPEED_RIGHT_INDEX]];
				gs.ball.ballPhysics.allSpeeds[SPEED_RIGHT_INDEX] = BRAKES_FRICTION[tmp];

				//give the player's momentem to ball if in dodge
				if (playerHitInDodge) {
					AddSpeeds(playerHit->playerPhysics.allSpeeds, gs.ball.ballPhysics.allSpeeds);
				}
			}
			else {
				//if the ball hits the player head "follow" player
				AddUint8Capped(&gs.ball.ballPhysics.allSpeeds[SPEED_LEFT_INDEX], playerHit->playerPhysics.allSpeeds[SPEED_LEFT_INDEX]);
				AddUint8Capped(&gs.ball.ballPhysics.allSpeeds[SPEED_RIGHT_INDEX], playerHit->playerPhysics.allSpeeds[SPEED_RIGHT_INDEX]);
			}

			//up and down
			tmp = gs.ball.ballPhysics.allSpeeds[SPEED_UP_INDEX];
			gs.ball.ballPhysics.allSpeeds[SPEED_UP_INDEX] = BRAKES_FRICTION[gs.ball.ballPhysics.allSpeeds[SPEED_DOWN_INDEX]];
			gs.ball.ballPhysics.allSpeeds[SPEED_DOWN_INDEX] = BRAKES_FRICTION[tmp];

			//bounce off the player's head
			AddUint8Capped(&gs.ball.ballPhysics.allSpeeds[SPEED_UP_INDEX], playerHit->playerPhysics.allSpeeds[SPEED_UP_INDEX] + PLAYER_HEAD_BALL_BOUNCE);

			//remove thrust
			BallStopThrust();

			//change ball allegiance if the player is dodging as a parry
			if (//check if the player hit is dodging
				playerHitInDodge
				&& !BallPlayerMatch
				) 
			{
				//change whos side the ball is on
				gs.ball.ballFlags = BitCopy(playerHit->playerFlags, PLAYER_SECOND, gs.ball.ballFlags, BALL_ON_PLAYER2);

				//make the ball not neutral
				FLAG_ZERO(gs.ball.ballFlags, BALL_NEUTRAL);

				//reset the players dodge
				playerHit->playerTimer[PLAYER_STUN_TIMER] = 0;
				playerHit->playerTimer[PLAYER_DODGE_TIMER] = 0;
				playerHit->playerTimer[PLAYER_BOUNCH_TIMER] = 0;
				playerHit->playerTimer[PLAYER_SOLID_TIMER] = 0;
				playerHit->playerTimer[PLAYER_INVISIBILITY_TIMER] = 0;

			} //end of parry dodge
		}

		//ball bounce regulation
		FLAG_SET(gs.ball.ballFlags, BALL_INSIDE_PLAYER);
		gs.ball.ballTimers[BALL_PLAYER_BOUNCE_IGNOR] = BALL_TIMER_PLAYER_IGNORE_BOUNCE; //don't bonce off player if you did it too recently

	}else{
		FLAG_ZERO(gs.ball.ballFlags, BALL_INSIDE_PLAYER);
	}
}

//where most of the action happens
void BallStep(void){

	//if not on a player
	if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER) == false) {

		//run this first before something turns it off
		BallTestCharged();

		//ball vs player
		BallInsidePlayer();

		//make this safe again
		BallTestNeutral();

		//thrust
		RocketBall();

		//move the ball
		PhysicsStep(&gs.ball.ballPhysics);

		//run after physics to find true speed
		BallTestTooFast();

	}
	else //else ball is on player
	{
		//set ball on player
		FLAG_SET(gs.ball.ballFlags, BALL_INSIDE_PLAYER);

		//remove ball's parry if held for too long
		if (0 == gs.ball.ballTimers[BALL_PARRY_TIMER_TIMEOUT]) {
			FLAG_ZERO(gs.ball.ballFlags, BALL_CHARGED);
		}
	}

	DiscernmentAllTimers(gs.ball.ballTimers, BALL_TIMER_COUNT);

	//make ball bounce sounds off walls/floor
	if (SpeedCompare(gs.ball.ballPhysics.allSpeeds) > TO_FIXPOINT(SEFFECTS_BALL_MIN_SPEED) //if the ball is going fast enough
		&& FLAG_TEST(gs.ball.ballPhysics.physicsFlags, PHYSICS_IN_WALL)) { //if its in a wall

		//loop though ball sound effects
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

void DrawBallDebug(void) {

	if (FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL)) {
		SDL_SetRenderDrawColor(mainRenderer, BALL_NEUTRAL_DEBUG_R, BALL_NEUTRAL_DEBUG_G, BALL_NEUTRAL_DEBUG_B, 0xFF);
	}
	else { //ball is armed and deadly
		if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2)) {
			SDL_SetRenderDrawColor(mainRenderer, PLAYER2_RED_DEBUG, PLAYER2_GREEN_DEBUG, PLAYER2_BLUE_DEBUG, 0xFF);
		}
		else {
			SDL_SetRenderDrawColor(mainRenderer, PLAYER1_RED_DEBUG, PLAYER1_GREEN_DEBUG, PLAYER1_BLUE_DEBUG, 0xFF);
		}
	}

	SDL_Rect tmp = BoxToRec(&gs.ball.ballPhysics.postionWorldSpace);
	//draw on player if player owned
	if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER)) {
		tmp.x = REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.x);
		tmp.y = REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.y);
		if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2)) {
			tmp.x = REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.x);
			tmp.y = REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.y);
		}
		tmp.y += tmp.h >> 1;
	}
	//draw box
	if ((FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER) && FLAG_TEST(gs.ball.ballFlags, BALL_CHARGED))
		|| (FLAG_TEST(gs.ball.ballFlags, BALL_TOO_FAST) && !FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL))) {
		SDL_RenderFillRect(mainRenderer, &tmp);
	}
	else {
		SDL_RenderDrawRect(mainRenderer, &tmp);
	}

}

void DrawBall(void) {

	//don't draw ball on player if they are holding it 
	if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER)) {
		return;
	}

	int16_t xOffset = TO_FIXPOINT(SPRITE_WIDTH_FORTH);
	int16_t yOffset = TO_FIXPOINT(SPRITE_HEIGHT_FORTH);
	int16_t ballX = gs.ball.ballPhysics.postionWorldSpace.topLeft.x;
	int16_t ballY = gs.ball.ballPhysics.postionWorldSpace.topLeft.y;
	uint8_t index = SPRITE_INDEX_BALLSTILL;
	bool inverth = false;
	bool invertv = false;
	bool blink = false;

	const bool spawnSmokeRate = (gs.spriteTimer & 1) && (gs.spriteTimer & 2);
	const bool player2Col = FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2);

	//change ball sprite if its moving fast enought to hurt player
	if (FLAG_TEST(gs.ball.ballFlags, BALL_TOO_FAST) && !FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL)) {

		//get total hspeed
		const int16_t hspeed = (int16_t)gs.ball.ballPhysics.allSpeeds[SPEED_LEFT_INDEX] + (int16_t)gs.ball.ballPhysics.allSpeeds[SPEED_RIGHT_INDEX];

		if (hspeed != 0) { //find which has the greater magantude
			//vspeed
			inverth = gs.ball.ballPhysics.allSpeeds[SPEED_LEFT_INDEX] < gs.ball.ballPhysics.allSpeeds[SPEED_RIGHT_INDEX];
			index = SPRITE_INDEX_BALLHRZ;
		}
		else {
			//hspeed
			invertv = gs.ball.ballPhysics.allSpeeds[SPEED_UP_INDEX] > gs.ball.ballPhysics.allSpeeds[SPEED_DOWN_INDEX];
			index = SPRITE_INDEX_BALLVERT;
		}

		//smoke rings
		if (spawnSmokeRate && gs.ball.ballTimers[BALL_PARRY_TIMER] != 0) {
			ParticleAdd(ballX - xOffset, ballY - yOffset, 0, 0, 0, -1, SPRITE_INDEX_SMOKERING, 15);
		}

		//sparks
		if (FLAG_TEST(gs.ball.ballPhysics.physicsFlags, PHYSICS_IN_WALL)) {
			for (uint8_t i = RngMasked8(RNG_MASK_15); i > 0; --i) {
				ParticleAdd(ballX - xOffset, ballY - yOffset, -32 + (5 + (int8_t)(RngMasked8(RNG_MASK_63))), -32 + (5 + (int8_t)(RngMasked8(RNG_MASK_63))), 0, 2, SPRITE_INDEX_SPARK, RngMasked8(RNG_MASK_15));
			}
		}
	}

	
	DrawSprite(ballX - xOffset, ballY - yOffset, index, true, inverth, invertv, blink, player2Col);
}

//
//end draw


#endif