#ifndef MAP_H
#define MAP_H


//init and load map before players or ball (map holds spawn locations)
void InitMap(void) {
	gs.mapBoxCount = 0;
	ZeroOut((uint8_t*)&gs.map, sizeof(gs.map));
	ZeroOut((uint8_t*)&gs.startSpawnMap, sizeof(gs.startSpawnMap));
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

void LoadMapArray(const uint16_t * const mapIn, uint16_t count) {
	//used by LoadMap

	//remove old map
	InitMap();

	//get spawn locaions
	for (uint8_t i = 0; i < MAP_SPAWN_COUNT; ++i) {
		gs.startSpawnMap[i] = mapIn[i];
	}

	//get count
	count = (count - MAP_SPAWN_COUNT) / 4;

	//x, y, h, w
	for (gs.mapBoxCount = 0; gs.mapBoxCount < count;) {
		LoadMapBlock(
			mapIn[gs.mapBoxCount * 4 + MAP_SPAWN_COUNT],
			mapIn[gs.mapBoxCount * 4 + 1 + MAP_SPAWN_COUNT],
			mapIn[gs.mapBoxCount * 4 + 2 + MAP_SPAWN_COUNT],
			mapIn[gs.mapBoxCount * 4 + 3 + MAP_SPAWN_COUNT]
		);
	}
}

void LoadMap(const uint8_t mapId) {

	if (mapId >= MAP_COUNT) {
#ifdef NDEBUG
		return;
#else
		printf("LoadMap mapId is OB");
		assert(false);
#endif
	}

	InitMap();
	gs.mapIndex = mapId;

	switch (gs.mapIndex)
	{
		case MAP_DEBUG:
			LoadMapArray(mapDebug, sizeof(mapDebug) / sizeof(uint16_t));
			LogTextScreen(MAP_NAME_DEBUG, 0);
		break;

		case MAP_EMPY:
			LoadMapArray(mapEmpy, sizeof(mapEmpy) / sizeof(uint16_t));
			LogTextScreen(MAP_NAME_EMPTY, 0);
			break;

		case MAP_BIG_S:
			LoadMapArray(mapBigS, sizeof(mapBigS) / sizeof(uint16_t));
			LogTextScreen(MAP_NAME_LINE, 0);
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

void SwitchMap(const uint8_t mapIndex) {
	//error checking
	if (mapIndex >= MAP_COUNT) {
#ifdef NDEBUG
		return;
#else
		printf("SwitchMap mapIndex is OB");
		assert(false);
#endif
	}

	newMapIndex = mapIndex;
}

//run in a step to do animation with MAP_NON as the map
//set switchMapIndex only when you want to switch maps
void SwitchMapSlideStep(const uint8_t switchMapIndex) {

	static uint8_t newMap = MAP_NON;
	static uint16_t mapAcc = 0;

	if (switchMapIndex != MAP_NON) {
		newMap = switchMapIndex;
	}

	if (newMap == MAP_NON) return;

	//move the graphics off screen
	if (newMapIndex != newMap 
		&& spriteOffsetX < BASE_RES_WIDTH) {
		SetSpriteOffsetRelative(mapAcc += MAP_SWITCH_ACC, 0);
	}

	//switch maps
	if (spriteOffsetX >= BASE_RES_WIDTH) {
		SwitchMap(newMap);
		SetSpriteOffset(-BASE_RES_WIDTH, spriteOffsetY);
	}

	//move map back after map switch 
	if (newMapIndex == newMap
		&& mapAcc > 0) {

		SetSpriteOffsetRelative((mapAcc -= MAP_SWITCH_ACC), 0);

		if (mapAcc <= 0) {
			SetSpriteOffset(0, spriteOffsetY);
			mapAcc = 0;
			gs.backgroundShakeRate = BACKGROUND_SHAKE_START_RATE;
		}
	}

}

#endif