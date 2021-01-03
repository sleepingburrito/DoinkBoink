#pragma once
#include "BaseCollision.h"
#include "GlobalState.h"

//updates the pointers in gloable state 
void InitBaseCollision(void) {
	for (uint8_t playerIndex = 0; playerIndex < PLAYER_COUNT; ++playerIndex) {
		playerHitBall[playerIndex] = NULL;
		playerHitBallInflate[playerIndex] = NULL;
		playerHitPlayer[playerIndex] = NULL;
	}

	BallHitBall = NULL;
	BallHitPlayer = NULL;
}

void UpdateBaseCollision(void) {
	
	//remove old collisions
	InitBaseCollision();

	//player to player
	bool playerOverlap = false;
	if (BoxOverlap(&gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace, 
		&gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace)) {
		playerHitPlayer[PLAYER_TWO] = &gs.players[PLAYER_ONE];
		playerHitPlayer[PLAYER_ONE] = &gs.players[PLAYER_TWO];
		playerOverlap = true;
	}

	//ball to player
	for (uint8_t playerIndex = 0; playerIndex < PLAYER_COUNT; ++playerIndex) {

		//ducking player hitbox
		boxWorldSpace playerHitbox = InitBox(gs.players[playerIndex].playerPhysics.postionWorldSpace.topLeft.x,
			gs.players[playerIndex].playerPhysics.postionWorldSpace.topLeft.y,
			gs.players[playerIndex].playerPhysics.postionWorldSpace.boxSize.height,
			gs.players[playerIndex].playerPhysics.postionWorldSpace.boxSize.width);
		//if the player is ducking shorten the box
		if (FLAG_TEST(gs.players[playerIndex].playerFlags, PLAYER_DUCKING)) { 
			const uint16_t newHeight = gs.players[playerIndex].playerPhysics.postionWorldSpace.boxSize.height >> PLAYER_HEIGHT_DUCKING_SHIFT;
			const uint16_t heightOffset = playerHitbox.boxSize.height - newHeight;
			playerHitbox.boxSize.height = newHeight;
			MoveBoxRelative(&playerHitbox, 0, heightOffset);
		}
		//infalted hitbox for catching
		const boxWorldSpace catchBox = InitBox(gs.players[playerIndex].playerPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(PLAYER_CATCH_INFLATE),
			gs.players[playerIndex].playerPhysics.postionWorldSpace.topLeft.y - TO_FIXPOINT(PLAYER_CATCH_INFLATE),
			TO_FIXPOINT(PLAYER_HEIGHT + (PLAYER_CATCH_INFLATE << 1)),
			TO_FIXPOINT(PLAYER_WIDTH + (PLAYER_CATCH_INFLATE << 1)));

		//normal ball to player hit
		if (BoxOverlap(&playerHitbox,
			&gs.ball.ballPhysics.postionWorldSpace)
			&& !(FLAG_TEST(gs.ball.ballFlags, BALL_INSIDE_PLAYER) //help remove bises of getting hit by ball by ignoring the player who is trowing when inside another player
				&& !FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL)
				&& FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2) == playerIndex
				&& playerOverlap)
			) {
			//tell eatch other they hit
			playerHitBall[playerIndex] = &gs.ball;
			BallHitPlayer = &gs.players[playerIndex];
		}
		//player catch hit
		if (BoxOverlap(&catchBox,
			&gs.ball.ballPhysics.postionWorldSpace)) {
			playerHitBallInflate[playerIndex] = &gs.ball;
		}

	}
}