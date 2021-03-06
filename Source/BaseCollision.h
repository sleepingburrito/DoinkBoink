#ifndef BASECOLLISION_H
#define BASECOLLISION_H


void InitBaseCollision(void) {
	//updates the pointers in gloable state 

	for (uint8_t playerIndex = 0; playerIndex < PLAYER_COUNT; ++playerIndex) {
		playerHitBall[playerIndex] = NULL;
		playerHitBallInflate[playerIndex] = NULL;
		playerHitPlayer[playerIndex] = NULL;
	}

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
	if (!FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER)) { //if the player is holding the ball collisions is disabled
		for (uint8_t playerIndex = 0; playerIndex < PLAYER_COUNT; ++playerIndex) {

			//used to change the order of the player index (to help remove the bias of who has the ball when throwing) 
			uint8_t playerIndexTmp = playerIndex;
			if (FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2)) {
				playerIndexTmp = !playerIndex;
			}

			//player hitbox
			boxWorldSpace playerHitbox = InitBox(gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.topLeft.x,
				gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.topLeft.y,
				gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.boxSize.height,
				gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.boxSize.width);

			//if the player is ducking shorten the box
			if (FLAG_TEST(gs.players[playerIndexTmp].playerFlags, PLAYER_DUCKING)) {
				const uint16_t newHeight = gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.boxSize.height >> PLAYER_HEIGHT_DUCKING_SHIFT;
				const uint16_t heightOffset = playerHitbox.boxSize.height - newHeight;
				playerHitbox.boxSize.height = newHeight;
				MoveBoxRelative(&playerHitbox, 0, heightOffset);
			}

			//normal ball to player hit
			if (
				BoxOverlap(&playerHitbox, &gs.ball.ballPhysics.postionWorldSpace)
				&& !(FLAG_TEST(gs.ball.ballFlags, BALL_INSIDE_PLAYER) //help remove biases of getting hit by ball by ignoring the player who is throwing when inside another player
					&& !FLAG_TEST(gs.ball.ballFlags, BALL_NEUTRAL)
					&& FLAG_TEST(gs.ball.ballFlags, BALL_ON_PLAYER2) == playerIndexTmp
					&& playerOverlap)
				) {
				//tell eatch other they hit
				playerHitBall[playerIndexTmp] = &gs.ball;
				BallHitPlayer = &gs.players[playerIndexTmp];
			}

			//inflate hitbox for catching
			const boxWorldSpace catchBox = InitBox(gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.topLeft.x - TO_FIXPOINT(PLAYER_CATCH_INFLATE),
				gs.players[playerIndexTmp].playerPhysics.postionWorldSpace.topLeft.y - TO_FIXPOINT(PLAYER_CATCH_INFLATE),
				TO_FIXPOINT(PLAYER_HEIGHT + (PLAYER_CATCH_INFLATE << 1)),
				TO_FIXPOINT(PLAYER_WIDTH + (PLAYER_CATCH_INFLATE << 1)));
			//player catching check
			if (BoxOverlap(&catchBox,
				&gs.ball.ballPhysics.postionWorldSpace)) {
				playerHitBallInflate[playerIndexTmp] = &gs.ball;
			}

		}//end of for loop
	}//end of checking if the ball is on the player

}

#endif