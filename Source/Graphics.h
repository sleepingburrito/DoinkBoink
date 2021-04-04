#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL_image.h>


void SetFullScreen(const bool fullScreen) {
	if (window == NULL) return;

	if (SDL_SetWindowFullscreen(window, fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	SDL_ShowCursor(fullScreen ? SDL_DISABLE : SDL_ENABLE);
}

//turn game box data into a SDL_Rec
SDL_Rect BoxToRec(const boxWorldSpace * const testbox) {
	//helps draw drbug boxes
	SDL_Rect returnRec;
	returnRec.x = REMOVE_FIXPOINT(testbox->topLeft.x);
	returnRec.y = REMOVE_FIXPOINT(testbox->topLeft.y);
	returnRec.w = REMOVE_FIXPOINT(testbox->boxSize.width);
	returnRec.h = REMOVE_FIXPOINT(testbox->boxSize.height);
	return returnRec;
}

//call before calling other draw calls
void PrepRendering(void) {
	
	//render to a texture
	if (SDL_SetRenderTarget(mainRenderer, mainDrawTexture) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//mark reflection drawing done at end of frame
	drawReflection = false;
}

void ClearScreenSoildColor(void) {
	
	if (SDL_SetRenderDrawColor(mainRenderer, BASE_BG_COLOR, BASE_BG_COLOR, BASE_BG_COLOR, 0xFF) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	if (SDL_RenderClear(mainRenderer) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}
}


//--Text tools--

//Note: x/y are not fixpoint
void DrawText(uint16_t x, uint16_t y, const bool big, const char* text) {

	if (NULL == text) {
#ifdef NDEBUG
		return;
#else
		printf("null text DrawText \n");
		assert(false);
#endif
	}

	const uint16_t xStart = x;
	const uint16_t hight = big ? SPRITE_HEIGHT : SPRITE_TEXTSMALL_HEIGHT;
	const uint16_t width = big ? SPRITE_WIDTH : SPRITE_TEXTSMALL_WIDTH;

	SDL_Rect SrcR;
	SrcR.h = hight;
	SrcR.w = width;

	SDL_Rect DestR;
	DestR.h = hight;
	DestR.w = width;

	do {
		if (*text == '\n') { //new line
			y += hight;
			x = xStart;
			continue;
		}
		else if (*text >= ' ' && *text <= 'Z') { //drawing chars as sprites if in range

			//move to next char spot
			x += width;

			//space
			if (*text == ' ') continue;

			//draw char
			const int tempWidth = (*text - ' ') * width;
			SrcR.x = tempWidth % TEXTURE_WIDTH;
			SrcR.y = SPRITE_HEIGHT * (tempWidth / TEXTURE_WIDTH);

			DestR.x = x;
			DestR.y = y;

			SDL_RenderCopyEx(mainRenderer, big ? spriteTextTexBig : spriteTextTexSmall, &SrcR, &DestR, 0, NULL, SDL_FLIP_NONE);
		}
	} while (*(text++));
}

//Note: not fixed point
void DrawTextNumberAppend(const uint16_t x, const uint16_t y, const bool big, const char* text, int32_t number) {
	
	if (text == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("null text DrawTextNumberAppend \n");
		assert(false);
#endif
	}
	
	//draw starting text
	DrawText(x, y, big, text);
	
	//find ending x/y of text (where number will be drawn)
	const uint16_t width = big ? SPRITE_WIDTH : SPRITE_TEXTSMALL_WIDTH;
	const uint16_t hight = big ? SPRITE_HEIGHT : SPRITE_TEXTSMALL_HEIGHT;
	uint16_t xOffset = x;
	uint16_t yOffset = y;
	uint16_t i = 0;

	while (text[i]) {
		if (text[i] == '\n') {
			xOffset = x;
			yOffset += hight;
		}
		else if (text[i] >= ' ' && text[i] <= 'Z') {
			xOffset += width;
		}
		++i;
	}

	//draw a negative 
	if (number < 0) {
		number *= -1;
		DrawText(x, y, big, "-");
		xOffset += width;
	}

	//draw number
	if (0 == number) {
		DrawText(xOffset, yOffset, big, "0");
	}else {
		int32_t flippedNumber = 0;
		uint8_t places = 0; //used to know the number of 10s places when drawing

		//flip number around for drawing
		while (true) {
			flippedNumber += number % 10;
			number /= 10;
			++places;
			if (number > 0) {
				flippedNumber *= 10;
				continue;
			}
			break;
		}

		//draw numbers
		while (places--) {
			char num[2];
			num[1] = 0;
			num[0] = '0' + (flippedNumber % 10);
			DrawText(xOffset, yOffset, big, num);
			xOffset += width;
			flippedNumber /= 10;
		}
	}

}

//--end of text tools--


//x and y should be fixed point
void DrawSpriteReflection(const float reflectionYaxis) {
	reflectionLine = reflectionYaxis;
	drawReflection = true;
}

void DrawSprite(
	const uint16_t x, 
	const uint16_t y, 
	const uint16_t index, 
	const bool fastPlayback, 
	const bool flipH,
	const bool flipV,
	const bool blink,
	const bool player2Color) {
	
	if (index >= SPRITE_INDEX_COUNT) {
#ifdef NDEBUG
		// nondebug
		return;
#else
		printf("draw sprite index ob");
		assert(false);
#endif
	}

	if (x >= TO_FIXPOINT(BASE_RES_WIDTH) || y >= TO_FIXPOINT(BASE_RES_HEIGHT)) {
#ifdef NDEBUG
		// nondebug
		return;
#else
		printf("draw sprite cords ob");
		assert(false);
#endif
	}

	//set playback speed
	int8_t speed = SPRITE_SPEED_NORMAL;
	if (fastPlayback) {
		speed = SPRITE_SPEED_FAST;
	}

	//what frame to play
	int8_t frameOffset = ((gs.spriteTimer >> speed) & 1);

	//make player 2 always on the opposite frame
	if (player2Color) {
		frameOffset = !frameOffset;
	}

	//blinking effect
	if (blink && frameOffset) {
		SDL_SetTextureColorMod(spriteTex, SPRITE_COLOR_BLINK_R, SPRITE_COLOR_BLINK_G, SPRITE_COLOR_BLINK_B);
	};

	if (player2Color) {
		SDL_SetTextureColorMod(spriteTex, SPRITE_COLOR_ALT_R, SPRITE_COLOR_ALT_G, SPRITE_COLOR_ALT_B);
		if (blink && frameOffset) {
			SDL_SetTextureColorMod(spriteTex, SPRITE_COLOR_BLINK_ALT_R, SPRITE_COLOR_BLINK_ALT_G, SPRITE_COLOR_BLINK_ALT_B);
		}
	}

	//sprite flipping
	int8_t flipFlag = 0;
	if (flipH) {
		flipFlag = SDL_FLIP_HORIZONTAL;
	}
	if (flipV) {
		flipFlag |= SDL_FLIP_VERTICAL;
	}

	//draw sprite
	SDL_Rect SrcR;
	SrcR.x = frameOffset * SPRITE_WIDTH;
	SrcR.y = SPRITE_HEIGHT * index;
	SrcR.h = SPRITE_HEIGHT;
	SrcR.w = SPRITE_WIDTH;

	SDL_Rect DestR;
	DestR.x = REMOVE_FIXPOINT(x);
	DestR.y = REMOVE_FIXPOINT(y);
	DestR.h = SPRITE_HEIGHT;
	DestR.w = SPRITE_WIDTH;

	SDL_RenderCopyEx(mainRenderer, spriteTex, &SrcR, &DestR, 0, NULL, (SDL_RendererFlip)flipFlag);

	//draw reflections
	if (drawReflection) {

		//sprite flip
		flipFlag = 0;
		if (flipH) {
			flipFlag = SDL_FLIP_HORIZONTAL;
		}
		if (!flipV) {
			flipFlag |= SDL_FLIP_VERTICAL;
		}

		//find new x and y
		float yR = (reflectionLine - ((float)REMOVE_FIXPOINT(y) + SPRITE_HEIGHT)) * REFLECTION_MUTIPLYER + reflectionLine;
		DestR.y = (int)yR;

		if (DestR.y < BASE_RES_HEIGHT) {
			//draw
			SDL_SetTextureAlphaMod(spriteTex, REFLECTION_ALPHA);
			SDL_RenderCopyEx(mainRenderer, spriteTex, &SrcR, &DestR, 0, NULL, (SDL_RendererFlip)flipFlag);
			SDL_SetTextureAlphaMod(spriteTex, UINT8_MAX);
		}
	}

	//undo sprite color settings
	SDL_SetTextureColorMod(spriteTex, 0xFF, 0xFF, 0xFF);
}

//ending draw call to put the graphics on screen
void DrawRenderToScreen(void) {

	//change the draw target to the window
	if (SDL_SetRenderTarget(mainRenderer, NULL) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//clear area outside of main drawing area
	ClearScreenSoildColor();

	//copy the draw buffer to the main draw buffer used for the window
	if (SDL_RenderCopy(mainRenderer, mainDrawTexture, NULL, NULL) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//draw to window for user to see
	SDL_RenderPresent(mainRenderer);
}

//used to draw diagnostic text
void DrawTextStandAlone(const uint16_t x, const uint16_t y, const char* text) {
	//only call once you have loaded the small font
	//used to draw text to the screen doing the full render cycle
	//useful for showing text during loading or debugging
	PrepRendering();
	ClearScreenSoildColor();
	DrawText(x, y, false, text);
	DrawRenderToScreen();
}

//effects
void DrawBoxShadow(const boxWorldSpace* const boxIn) {
	//draws a shadow under a box on the current loaded map

	const int16_t boxCenterX = boxIn->topLeft.x - (boxIn->boxSize.width >> 1);

	//find the closets ground on the loaded map
	int16_t closetsY = -1;
	int16_t distY = UINT8_MAX;
	for (uint8_t i = 0; i < MAP_MAX_BLOCKS; ++i) {
		const int16_t tmpY = REMOVE_FIXPOINT((int16_t)gs.map[i].topLeft.y - (int16_t)boxIn->bottomRight.y);
		if (tmpY >= 0 //if its not under
			&& tmpY < UINT8_MAX //if its not too far away
			&& tmpY < distY //if its less than what I already found
			&& boxIn->topLeft.x < gs.map[i].bottomRight.x //if its between its sides
			&& boxIn->bottomRight.x > gs.map[i].topLeft.x
			) {
			closetsY = gs.map[i].topLeft.y;
			distY = tmpY;
		}
	}

	if (closetsY) {
		//add offsets
		closetsY = closetsY - (TO_FIXPOINT(SPRITE_HEIGHT) - SHADOW_OFFSET_Y);

		//make sure sprite will not draw off screen
		if ( closetsY > TO_FIXPOINT(SPRITE_HEIGHT) && closetsY < TO_FIXPOINT(BASE_RES_HEIGHT - SPRITE_HEIGHT)
			&&boxCenterX > 0 && boxCenterX < TO_FIXPOINT(BASE_RES_WIDTH - SPRITE_WIDTH) ) {
			//draw the shadow
			const int8_t alpha = UINT8_MAX - (uint8_t)distY;
			SDL_SetTextureAlphaMod(spriteTex, alpha);
			DrawSprite(boxCenterX, closetsY, SPRITE_INDEX_SHADOW, false, false, false, false, false);
			
		}
	}

	SDL_SetTextureAlphaMod(spriteTex, UINT8_MAX);
}


//--log text--
//
//note if the number is zero it will not draw it
void LogTextScreen(const char * const text, const int32_t number) {
	for (uint8_t i = 0; i < TEXT_LOG_LINES; ++i) {
		//find a open space in text buffer
		if (0 == textLogBuffer[i].timer) {
			//copy number over
			textLogBuffer[i].number = number;
			//copy text over
			for (uint8_t i2 = 0; i2 < TEXT_LOG_CHARS && text[i2] != 0; ++i2) {
				textLogBuffer[i].text[i2] = text[i2];
			}
			//set the timer
			textLogBuffer[i].timer = TEXT_LOG_TIME;
			break;
			//end of copy text
		}
		//end of open space
	}
}

void LogTextScreenTickTimers(void) {
	for (uint8_t i = 0; i < TEXT_LOG_LINES; ++i) {
		//find timers that need to be ticked
		if (textLogBuffer[i].timer != 0) {
			//tick them
			textLogBuffer[i].timer--;
			//check if that was the last tick
			if (0 == textLogBuffer[i].timer) {
				//zero out number
				textLogBuffer[i].number = 0;
				//zero out text
				for (uint8_t i2 = 0; i2 < TEXT_LOG_CHARS; ++i2) {
					textLogBuffer[i].text[i2] = 0;
				}
			}//end of last tick
		}//end finding timers
	}
}

void LogTextScreenDraw(void) {
	uint16_t texty = TEXT_LOG_Y;

	for (uint8_t i = 0; i < TEXT_LOG_LINES; ++i) {
		//find text to draw
		if (textLogBuffer[i].timer != 0) {
			
			//blink text
			if (!(textLogBuffer[i].timer < (uint8_t)(TEXT_LOG_TIME * TEXT_LOG_BLINK) && gs.spriteTimer & PLAYER_BLINK_RATE)) {
				//draw text
				if (textLogBuffer[i].number == 0) {
					DrawText(TEXT_LOG_X, texty, false, textLogBuffer[i].text);
				}
				else {
					DrawTextNumberAppend(TEXT_LOG_X, texty, false, textLogBuffer[i].text, textLogBuffer[i].number);
				}
			}	
			//move down a space
			texty += SPRITE_TEXTSMALL_HEIGHT + TEXT_LOG_Y_BUFFER;
		}//end timer check loop
	}
}
//
//--end log text--

void SpriteTimerTick(void) {
	gs.spriteTimer++;
}

//also loads background sprite sheet
void LoadSprites(void) {

	//small font (needs to load first for text support)
	char* tmpDir = BufferStringMakeBaseDir(SPRITE_TEXTSMALL_FILE);
	SDL_Surface* const TextSmallBufferSur = IMG_Load(tmpDir);
	if (NULL == TextSmallBufferSur) {
		printf("Failed to load %s\n", tmpDir);
		assert(false);
	}
	spriteTextTexSmall = SDL_CreateTextureFromSurface(mainRenderer, TextSmallBufferSur);
	if (NULL == spriteTextTexSmall) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}
	
	//draw loading screen
	ClearScreenSoildColor();
	DrawTextStandAlone(LOADING_TEXT_X, LOADING_TEXT_Y, LOADING_TEXT_SPRITES);

	//player sprites
	tmpDir = BufferStringMakeBaseDir(SPRITE_FILE);
	SDL_Surface* const SpriteBufferSur = IMG_Load(tmpDir);
	if (NULL == SpriteBufferSur) {
		printf("Failed to load %s\n", tmpDir);
		assert(false);
	}

	//large font
	tmpDir = BufferStringMakeBaseDir(SPRITE_TEXTBIG_FILE);
	SDL_Surface* const TextBigBufferSur = IMG_Load(tmpDir);
	if (NULL == TextBigBufferSur) {
		printf("Failed to load %s\n", tmpDir);
		assert(false);
	}

	//backgrounds
	tmpDir = BufferStringMakeBaseDir(BACKGROUND_FILE);
	SDL_Surface* const BackgroundBufferSur = IMG_Load(tmpDir);
	if (NULL == BackgroundBufferSur) {
		printf("Failed to load %s\n", tmpDir);
		assert(false);
	}

	//make textures
	spriteTex = SDL_CreateTextureFromSurface(mainRenderer, SpriteBufferSur);
	spriteTextTexBig = SDL_CreateTextureFromSurface(mainRenderer, TextBigBufferSur);
	backgrounds = SDL_CreateTextureFromSurface(mainRenderer, BackgroundBufferSur);

	//enable transparency for sprites
	if (SDL_SetTextureBlendMode(spriteTex, SDL_BLENDMODE_BLEND) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//make sure all textures loaded
	if (NULL == spriteTex
		|| NULL == spriteTextTexBig
		|| NULL == backgrounds) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//free temp buffers
	SDL_FreeSurface(SpriteBufferSur);
	SDL_FreeSurface(TextBigBufferSur);
	SDL_FreeSurface(TextSmallBufferSur);
	SDL_FreeSurface(BackgroundBufferSur);
}

//Note: this also closes all of SDL
void ShutdownWindow(void) {
	IMG_Quit();
	SDL_DestroyTexture(mainDrawTexture);
	
	SDL_DestroyTexture(spriteTex);
	SDL_DestroyTexture(spriteTextTexBig);
	SDL_DestroyTexture(spriteTextTexSmall);
	SDL_DestroyTexture(backgrounds);

	SDL_DestroyRenderer(mainRenderer);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
	SDL_Quit();
}


//--map graphics--
//
void DrawMapDebug(void) {
	SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0, 0, 0xFF);
	for (uint8_t i = 0; i < gs.mapBoxCount; ++i) {
		//draw 2 pixel wide rectangle
		SDL_Rect rec = BoxToRec(&gs.map[i]);
		SDL_RenderDrawRect(mainRenderer, &rec);//rec1
		rec.x++;
		rec.y++;
		rec.h -= 2;
		rec.w -= 2;
		SDL_RenderDrawRect(mainRenderer, &rec);//rec2
	}
}

void InitPointLight(void) {
	for (uint16_t i = 0; i < POINT_LIGHT_MAX_BOXES; ++i) {
		lightBoxs[i].w = POINTLIGHT_BOX_WIDTH;
		lightBoxs[i].h = POINTLIGHT_BOX_HEIGTH;
	}
}

void DrawPointLight(void) {

	//setup the boxes
	int32_t lightBoxCount = 0;

	//get shadow casting box zones
	const float box0left = (float)REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.x);
	const float box0right = (float)REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.bottomRight.x);
	float box0top = (float)REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.topLeft.y);
	float box0bottom = (float)REMOVE_FIXPOINT(gs.players[PLAYER_ONE].playerPhysics.postionWorldSpace.bottomRight.y);

	if (FLAG_TEST(gs.players[PLAYER_ONE].playerFlags, PLAYER_DUCKING)) {
		box0top += SPRITE_HEIGHT;
		box0bottom = box0top + SPRITE_WIDTH_HALF;
	}

	const float box1left = (float)REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.x);
	const float box1right = (float)REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.bottomRight.x);
	float box1top = (float)REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.topLeft.y);
	float box1bottom = (float)REMOVE_FIXPOINT(gs.players[PLAYER_TWO].playerPhysics.postionWorldSpace.bottomRight.y);

	if (FLAG_TEST(gs.players[PLAYER_TWO].playerFlags, PLAYER_DUCKING)) {
		box1top += SPRITE_HEIGHT;
		box1bottom = box1top + SPRITE_WIDTH_HALF;
	}

	const float box2left = (float)REMOVE_FIXPOINT(gs.ball.ballPhysics.postionWorldSpace.topLeft.x);
	const float box2right = (float)REMOVE_FIXPOINT(gs.ball.ballPhysics.postionWorldSpace.bottomRight.x);
	const float box2top = (float)REMOVE_FIXPOINT(gs.ball.ballPhysics.postionWorldSpace.topLeft.y);
	const float box2bottom = (float)REMOVE_FIXPOINT(gs.ball.ballPhysics.postionWorldSpace.bottomRight.y);

	//trace light rays
	for (int32_t x = POINTLIGHT_LEFT_MAX; x < POINTLIGHT_RIGHT_MAX; ++x) {

		const float fx = (float)x;
		const double subX = POINT_LIGHT_X_BOXSPACE - (double)fx;
		const double doubleX = POW2(subX);

		for (int32_t y = POINTLIGHT_TOP_MAX; y < POINTLIGHT_BOTTOM_MAX; ++y) {

			//loop setup
			bool notInObj = true;

			const float fy = (float)y;
			const double subY = POINT_LIGHT_Y_BOXSPACE - (double)fy;
			const double angle = (float)atan2(subY, subX);
			const float cosSave = (float)cos(angle);
			const float sinSave = (float)sin(angle);
			const float dist = (float)sqrt(doubleX + POW2(subY));

			//if outside of light cicle
			if (dist > POINTLIGHT_DIST_MAX_BOXSPACE) {
				if (y > POINTLIGHT_BOTTOM_MAX / 2) {
					//if your outside your circle move onto the next row
					break;
				}
				else {
					continue;
				}
			}

			for (float i = 0; i < dist && notInObj; ++i) {

				const float xCheck = (fx + cosSave * i) * POINTLIGHT_BOX_WIDTH + POINTLIGHT_BOX_WIDTH_HALF;
				const float yCheck = (fy + sinSave * i) * POINTLIGHT_BOX_HEIGTH + POINTLIGHT_BOX_HEIGTH_HALF;


				//test if a point is in a box
				notInObj = (
					!(xCheck > box0left && xCheck < box0right&& yCheck > box0top && yCheck < box0bottom)
					&& !(xCheck > box1left && xCheck < box1right&& yCheck > box1top && yCheck < box1bottom)
					&& !(xCheck > box2left && xCheck < box2right&& yCheck > box2top && yCheck < box2bottom)
					);

			}

			//draw light
			if (notInObj) {
				lightBoxs[lightBoxCount].x = x * POINTLIGHT_BOX_WIDTH;
				lightBoxs[lightBoxCount].y = y * POINTLIGHT_BOX_HEIGTH;
				lightBoxCount++;
			}

		}//end of y loop
	}//end of x loop

	//draw light
	const uint8_t flameFlicker = ((uint8_t)(gs.spriteTimer ^ gs.rngSeed ^ lightBoxCount) < 5 || gs.spriteTimer < 4) ? POINT_LIGHT_FLICKER_OFFSET : 0;
	SDL_SetRenderDrawColor(mainRenderer, POINT_LIGHT_R, POINT_LIGHT_G - flameFlicker, POINT_LIGHT_B, POINT_LIGHT_A);
	SDL_RenderFillRects(mainRenderer, lightBoxs, lightBoxCount);
}

void DrawBackground(const uint8_t mapIndex) {

	if (mapIndex >= MAP_COUNT) {
#ifdef NDEBUG
		return;
#else
		printf("drawbackground mapIndex ob");
		assert(false);
#endif
	}

	int32_t xOffset = 0;
	static uint8_t backgroundShakeIndex = 0;

	//screen shake
	if (gs.backgroundShakeRate != 0) {

		if (gs.backgroundShakeRate <= BACKGROUND_SHAKE_DECAY_RATE) {
			gs.backgroundShakeRate = 0;
			backgroundShakeIndex = 0;
		}
		else {
			backgroundShakeIndex += gs.backgroundShakeRate;
			gs.backgroundShakeRate -= BACKGROUND_SHAKE_DECAY_RATE;
		}

		xOffset = SIN_TABLE[backgroundShakeIndex] / BACKGROUND_SHAKE_DIV;
	}

	SDL_Rect SrcR;
	SrcR.x = 0;
	SrcR.y = (int)mapIndex * BASE_RES_HEIGHT;
	SrcR.h = BASE_RES_HEIGHT;
	SrcR.w = BASE_RES_WIDTH;

	SDL_Rect DestR;
	DestR.x = xOffset;
	DestR.y = 0;
	DestR.h = BASE_RES_HEIGHT;
	DestR.w = BASE_RES_WIDTH;

	SDL_RenderCopyEx(mainRenderer, backgrounds, &SrcR, &DestR, 0, NULL, SDL_FLIP_NONE);

	//draw map specific graphics
	switch (gs.mapIndex) {
	case MAP_EMPY:
		DrawSpriteReflection(REFLECTION_LINE);
		break;

	case MAP_BIG_S:
		DrawPointLight();
		//DrawMapDebug();
		break;
	}

}

void DrawForeground(void) {

}
//
//--end of map graphics--


void InitWindow(void) {
	//start video
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	window = SDL_CreateWindow(GAME_NAME " " GAME_VS, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, BASE_RES_WIDTH, BASE_RES_HEIGHT, SDL_WINDOW_SHOWN);
	if (NULL == window) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//Allow window scaling
	SDL_SetWindowResizable(window, SDL_TRUE);

	windowSurface = SDL_GetWindowSurface(window);
	if (NULL == windowSurface) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//vsynce here and make main renderer
	mainRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED GFX_VSYNCE);
	if (NULL == mainRenderer) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//set alpha on
	if (SDL_SetRenderDrawBlendMode(mainRenderer, SDL_BLENDMODE_BLEND) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//creat main renderer
	mainDrawTexture = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BASE_RES_WIDTH, BASE_RES_HEIGHT);
	if (mainDrawTexture == NULL) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//load the png loader
	if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG)) {
		printf("SDL_image Error: %s\n", IMG_GetError());
		assert(false);
	}

	//set it so the window keeps the same aspect ratio when resizing
	if (0 != SDL_RenderSetLogicalSize(mainRenderer, BASE_RES_WIDTH, BASE_RES_HEIGHT)) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//set smooth scaling
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") != SDL_TRUE) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//Get the display refresh rate
	for (uint8_t i = 0; i < SDL_GetNumVideoDisplays(); ++i) {

		SDL_DisplayMode current;

		if (SDL_GetCurrentDisplayMode(i, &current) != 0) {
			printf("Could not get display mode %s\n", SDL_GetError());
			assert(false);
		}
		else if (current.refresh_rate > displayRefreshRate) {
			displayRefreshRate = (uint16_t)current.refresh_rate;
		}
	}

	//etc
	InitPointLight();
	ZeroOut((uint8_t*)textLogBuffer, sizeof(textLogBuffer));
	LoadSprites();
	gs.spriteTimer = 0;
	reflectionLine = 0;
	drawReflection = false;
}

#endif