#pragma once
#include "Tools.h"
#include <stdint.h>
#include <stdbool.h>
#include "GlobalState.h"
#include "Const.h"
#include "typedefs.h"


//init and load map before players or ball (map holds spawn locations)
void InitMap(void) {
	//use to reset all map settings
	gs.mapBoxCount = 0;
	ZeroOut((uint8_t*)&gs.map, sizeof(gs.map));
}

bool MapTestInWall(const boxWorldSpace * const checkMe){
	if (checkMe == NULL) {
#ifdef NDEBUG
		return false;
#else
		printf("Null ptr");
		assert(false);
#endif
	}

	for (uint8_t i = 0; i < gs.mapBoxCount; ++i) {
		if (BoxOverlap(checkMe, &gs.map[i])) {
			return true;
		}
	}
	return false;
}

//note that args are not in fixed point
void LoadMapBlock(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height) {

	//error checking
	if (gs.mapBoxCount >= MAP_MAX_BLOCKS) {
#ifdef NDEBUG
		return;
#else
		printf("map array is too large. max is set to %d", MAP_MAX_BLOCKS);
		assert(false);
#endif
	}

	if (width < REMOVE_FIXPOINT(MAX_SPEED)) {
#ifdef NDEBUG
		return;
#else
		printf("box %d width is under min of %d", gs.mapBoxCount, MAX_SPEED);
		assert(false);
#endif
	}

	if (height < REMOVE_FIXPOINT(MAX_SPEED)) {
#ifdef NDEBUG
		return;
#else
		printf("box %d height is under min of %d", gs.mapBoxCount, MAX_SPEED);
		assert(false);
#endif
	}

	boxWorldSpace tmpBox = InitBox(
		TO_FIXPOINT(x),
		TO_FIXPOINT(y),
		TO_FIXPOINT(width),
		TO_FIXPOINT(height));

	if (MapTestInWall(&tmpBox)) {
#ifdef NDEBUG
		return;
#else
		printf("box %d overlaps another", gs.mapBoxCount);
		assert(false);
#endif
	}

	gs.map[gs.mapBoxCount++] = tmpBox;
}

void DrawMapDebug(void) {
	SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0, 0, 0xFF);
	for (uint8_t i = 0; i < gs.mapBoxCount; ++i) {
		//draw 2 pixel wide rec
		SDL_Rect rec = BoxToRec(&gs.map[i]);
		SDL_RenderDrawRect(mainRenderer, &rec);//rec1
		rec.x++;
		rec.y++;
		rec.h -= 2;
		rec.w -= 2;
		SDL_RenderDrawRect(mainRenderer, &rec);//rec2
	}
}

void LoadMapArray(const uint16_t * const mapIn, uint16_t count) {
	//get spawn locaions
	for (uint8_t i = 0; i < MAP_SPAWN_COUNT; ++i) {
		gs.startSpawnMap[i] = mapIn[i];
	}
	//get count
	count = (count - MAP_SPAWN_COUNT) / 4;
	//remove old map
	InitMap();
	//x, y, h, w
	for (gs.mapBoxCount = 0; gs.mapBoxCount < count;) {
		LoadMapBlock(mapIn[gs.mapBoxCount * 4 + MAP_SPAWN_COUNT],
			mapIn[gs.mapBoxCount * 4 + 1 + MAP_SPAWN_COUNT],
			mapIn[gs.mapBoxCount * 4 + 2 + MAP_SPAWN_COUNT],
			mapIn[gs.mapBoxCount * 4 + 3 + MAP_SPAWN_COUNT]);
	}
}

void LoadMap(const uint8_t mapId) {
	InitMap();
	gs.mapIndex = mapId;

	switch (gs.mapIndex)
	{
		case MAP_DEBUG:
			LoadMapArray(mapDebug, sizeof(mapDebug) / sizeof(uint16_t));
		break;

		case MAP_EMPY:
			LoadMapArray(mapEmpy, sizeof(mapEmpy) / sizeof(uint16_t));
			break;

		case MAP_BIG_S:
			LoadMapArray(mapBigS, sizeof(mapBigS) / sizeof(uint16_t));
			break;

		case MAP_TENNINS:
			LoadMapArray(mapTennis, sizeof(mapTennis) / sizeof(uint16_t));
			break;


	default:
#ifdef NDEBUG
		break;
#else
		printf("bad map id, id was %d", mapId);
		assert(false);
		break;
#endif
	}
}
