#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <float.h>
#include <stdint.h>
#include "typedefs.h"
#include "Tools.h"
#include "GlobalState.h"

//graphics settings
#define GFX_VSYNCE | SDL_RENDERER_PRESENTVSYNC

//starts SDL and opens the Window
void InitWindow(void) {
	//start video
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	window = SDL_CreateWindow(GAME_NAME " " GAME_VS, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, BASE_RES_WIDTH, BASE_RES_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL){
		printf("%s\n", SDL_GetError());
		assert(false);
	}
	SDL_SetWindowResizable(window, SDL_TRUE);
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	windowSurface = SDL_GetWindowSurface(window);
	if (windowSurface == NULL) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//vsynce here
	mainRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED GFX_VSYNCE);
	if (mainRenderer == NULL) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	mainDrawTexture = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BASE_RES_WIDTH, BASE_RES_HEIGHT);
	if (mainDrawTexture == NULL) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG)) {
		printf("SDL_image Error: %s\n", IMG_GetError());
		assert(false);
	}
}

void SetFullScreen(const bool fullScreen) {
	if (window == NULL) return;

	SDL_SetWindowFullscreen(window, fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SDL_ShowCursor(fullScreen ? SDL_DISABLE : SDL_ENABLE);
}

SDL_Rect BoxToRec(const boxWorldSpace * const testbox) {
	//helps draw drbug boxes
	SDL_Rect returnRec;
	returnRec.x = REMOVE_FIXPOINT(testbox->topLeft.x);
	returnRec.y = REMOVE_FIXPOINT(testbox->topLeft.y);
	returnRec.w = REMOVE_FIXPOINT(testbox->boxSize.width);
	returnRec.h = REMOVE_FIXPOINT(testbox->boxSize.height);
	return returnRec;
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

	//transprnt test debug
	//SDL_SetTextureBlendMode(backgrounds, SDL_BLENDMODE_BLEND);
	//SDL_SetTextureAlphaMod(backgrounds, 100);

	SDL_RenderCopyEx(mainRenderer, backgrounds, &SrcR, &DestR, 0, NULL, SDL_FLIP_NONE);
}

//call before calling other draw events
void PrepRendering(void) {
	
	//render to a texture
	if (SDL_SetRenderTarget(mainRenderer, mainDrawTexture)) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

}

void ClearScreenSoildColor(void) {
	//clear the screen
	//if (SDL_SetRenderDrawColor(mainRenderer, 200, 200, 200, 0xFF) != 0) { //light gray
	if (SDL_SetRenderDrawColor(mainRenderer, 40, 40, 40, 0xFF) != 0) { //near black
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	if (SDL_RenderClear(mainRenderer) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}
}

//x and y are not fixpoint
void DrawText(uint16_t x, uint16_t y, const bool big, const char* text) {

	if (text == NULL) {
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
	do {
		if (*text == '\n') { //new line
			y += hight;
			x = xStart;
			continue;
		}
		else if (*text >= ' ' && *text <= 'Z') { //drawing chars are sprites if in range
			//move to next spot
			x += width;
			//sapce
			if (*text == ' ') continue;
			//draw char
			SDL_Rect SrcR;
			SrcR.x = (int)(*text - ' ') * width;
			SrcR.y = 0;
			SrcR.h = hight;
			SrcR.w = width;

			SDL_Rect DestR;
			DestR.x = x;
			DestR.y = y;
			DestR.h = hight;
			DestR.w = width;

			SDL_RenderCopyEx(mainRenderer, big ? spriteTextTexBig : spriteTextTexSmall, &SrcR, &DestR, 0, NULL, SDL_FLIP_NONE);
		}
	} while (*(text++));
}

//not in fixed point
void DrawTextNumberAppend(const uint16_t x, const uint16_t y, const bool big, const char* text, int32_t number) {
	
	if (text == NULL) {
#ifdef NDEBUG
		return;
#else
		printf("null text DrawTextNumberAppend \n");
		assert(false);
#endif
	}
	
	//draw text part
	DrawText(x, y, big, text);
	
	//draw number, find ending locaion of text
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

	//draw nagative
	if (number < 0) {
		number *= -1;
		DrawText(x, y, big, "-");
		xOffset += width;
	}
	//draw number
	if (0 == number) {
		DrawText(xOffset, yOffset, big, "0");
	}
	else {
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

//x and y should be fixed point
void DrawSprite(const uint16_t x, const uint16_t y, 
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


	int8_t speed = SPRITE_SPEED_NORMAL;
	if (fastPlayback) {
		speed = SPRITE_SPEED_FAST;
	}
	int8_t frameOffset = ((gs.spriteTimer >> speed) & 1);

	//make player 2 always on the opposit frame
	if (player2Color) {
		frameOffset = !frameOffset;
	}

	if (blink && frameOffset) {
		SDL_SetTextureColorMod(spriteTex, 100, 100, 100);
	};

	if (player2Color) {
		SDL_SetTextureColorMod(spriteTex, 255, 40, 255);
		if (blink && frameOffset) {
			SDL_SetTextureColorMod(spriteTex, 0, 40, 255);
		}
	}

	int8_t flipFlag = 0;
	if (flipH) {
		flipFlag = SDL_FLIP_HORIZONTAL;
	}
	if (flipV) {
		flipFlag |= SDL_FLIP_VERTICAL;
	}

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

	//SDL_SetTextureColorMod(spriteTex, 0, 100, 0);
	//SDL_SetTextureColorMod(spriteTex, 0xFF, 0xFF, 0xFF);
	SDL_RenderCopyEx(mainRenderer, spriteTex, &SrcR, &DestR, 0, NULL, (SDL_RendererFlip)flipFlag);
	SDL_SetTextureColorMod(spriteTex, 0xFF, 0xFF, 0xFF);

}

//call after done drawing
void DrawRenderToScreen(void) {
	//draw texture on screen
	if (SDL_SetRenderTarget(mainRenderer, NULL) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	if (SDL_RenderCopy(mainRenderer, mainDrawTexture, NULL, NULL) != 0) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}

	//screen present
	SDL_RenderPresent(mainRenderer);
}

void DrawTextStandAlone(const uint16_t x, const uint16_t y, const char* text) {
	//only call once you have loaded the small font
	//used to draw text to the screen doing the full render cycle, usefull for showing text during loading
	PrepRendering();
	DrawText(x, y, false, text);
	DrawRenderToScreen();
}

//note if the number is zero it will not draw it
void LogTextScreen(const char * const text, const int32_t number) {
	for (uint8_t i = 0; i < TEXT_LOG_LINES; ++i) {
		//find a open space
		if (textLogBuffer[i].timer == 0) {
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
			
			textLogBuffer[i].timer--;

			//check if that was the last tic
			if (0 == textLogBuffer[i].timer) {
				//zero out number
				textLogBuffer[i].number = 0;
				//zero out text
				for (uint8_t i2 = 0; i2 < TEXT_LOG_CHARS; ++i2) {
					textLogBuffer[i].text[i2] = 0;
				}
			}
			//end of last tick

		}
		//end finding timers
	}
}

void LogTextScreenDraw(void) {
	uint16_t texty = TEXT_LOG_Y;

	for (uint8_t i = 0; i < TEXT_LOG_LINES; ++i) {
		//find text to draw
		if (textLogBuffer[i].timer != 0) {
			
			//draw text
			if (textLogBuffer[i].number == 0) {
				DrawText(TEXT_LOG_X, texty, false, textLogBuffer[i].text);
			}
			else {
				DrawTextNumberAppend(TEXT_LOG_X, texty, false, textLogBuffer[i].text, textLogBuffer[i].number);
			}

			texty += SPRITE_TEXTSMALL_HEIGHT + TEXT_LOG_Y_BUFFER;
		}
	}
}


//also loads background sprite sheet
void LoadSprites(void) {

	//small font (load first for text support)
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
	//larg font
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

	if (NULL == spriteTex
		|| NULL == spriteTextTexBig
		//|| NULL == spriteTextTexSmall
		|| NULL == backgrounds) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}


	SDL_FreeSurface(SpriteBufferSur);
	SDL_FreeSurface(TextBigBufferSur);
	SDL_FreeSurface(TextSmallBufferSur);
	SDL_FreeSurface(BackgroundBufferSur);
}


//note: this also closes all of SDL
void ShutdownWindow(void) {
	IMG_Quit();
	SDL_DestroyTexture(mainDrawTexture);
	SDL_DestroyTexture(spriteTex);
	SDL_DestroyRenderer(mainRenderer);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
	SDL_Quit();
}