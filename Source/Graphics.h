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

//also loads background sprite sheet
void LoadSprites(void) {

	SDL_Surface* const SpriteBufferSur = IMG_Load(SPRITE_FILE);
	SDL_Surface* const TextBigBufferSur = IMG_Load(SPRITE_TEXTBIG_FILE);
	SDL_Surface* const TextSmallBufferSur = IMG_Load(SPRITE_TEXTSMALL_FILE);
	SDL_Surface* const BackgroundBufferSur = IMG_Load(BACKGROUND_FILE);
	

	if (NULL == SpriteBufferSur) {
		printf("Failed to load \n" SPRITE_FILE);
		assert(false);
	}
	if (NULL == TextBigBufferSur) {
		printf("Failed to load \n" SPRITE_TEXTBIG_FILE);
		assert(false);
	}
	if (NULL == TextSmallBufferSur) {
		printf("Failed to load \n" SPRITE_TEXTSMALL_FILE);
		assert(false);
	}
	if (NULL == BackgroundBufferSur) {
		printf("Failed to load \n" BACKGROUND_FILE);
		assert(false);
	}


	spriteTex = SDL_CreateTextureFromSurface(mainRenderer, SpriteBufferSur);
	spriteTextTexBig = SDL_CreateTextureFromSurface(mainRenderer, TextBigBufferSur);
	spriteTextTexSmall = SDL_CreateTextureFromSurface(mainRenderer, TextSmallBufferSur);
	backgrounds = SDL_CreateTextureFromSurface(mainRenderer, BackgroundBufferSur);


	if (NULL == spriteTex 
		|| NULL == spriteTextTexBig 
		|| NULL == spriteTextTexSmall
		|| NULL == backgrounds) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}


	SDL_FreeSurface(SpriteBufferSur);
	SDL_FreeSurface(TextBigBufferSur);
	SDL_FreeSurface(TextSmallBufferSur);
	SDL_FreeSurface(BackgroundBufferSur);
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

	SDL_Rect SrcR;
	SrcR.x = 0;
	SrcR.y = (int)mapIndex * BASE_RES_HEIGHT;
	SrcR.h = BASE_RES_HEIGHT;
	SrcR.w = BASE_RES_WIDTH;

	SDL_Rect DestR;
	DestR.x = 0;
	DestR.y = 0;
	DestR.h = BASE_RES_HEIGHT;
	DestR.w = BASE_RES_WIDTH;

	SDL_RenderCopyEx(mainRenderer, backgrounds, &SrcR, &DestR, 0, NULL, SDL_FLIP_NONE);
}

//call before calling other draw events
void PrepRendering(void) {
	
	//render to a texture
	if (SDL_SetRenderTarget(mainRenderer, mainDrawTexture)) {
		printf("%s\n", SDL_GetError());
		assert(false);
	}
	
	//clear the screen
	//if (SDL_SetRenderDrawColor(mainRenderer, 200, 200, 200, 0xFF) != 0) { //light gray
	if (SDL_SetRenderDrawColor(mainRenderer, 180, 180, 180, 0xFF) != 0) {
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