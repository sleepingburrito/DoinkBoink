#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <assert.h>
#include <stdbool.h>
#include <float.h>
#include <stdint.h>
#include "typedefs.h"
#include "Tools.h"
#include "GlobalState.h"
#include "Graphics.h"

void InitSound(void) {
    //init SDL_mixer
    //https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_frame.html
    const int init = MIX_INIT_FLAC | MIX_INIT_OGG;
    if (Mix_Init(init) != init) {
        printf("%s\n", "Mix_Init: Failed to init");
        assert(false);
    }
    // start SDL with audio support
    //https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_frame.html
    if (SDL_Init(SDL_INIT_AUDIO) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError());
        assert(false);
    }
    // open 44.1KHz, signed 16bit, system byte order,
    //      stereo audio, using 1024 byte chunks
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("%s", Mix_GetError());
        assert(false);
    }

    //set defaults
    ZeroOut((uint8_t*)gameMusic, sizeof(gameMusic));
    ZeroOut((uint8_t*)gameSoundEffects, sizeof(gameSoundEffects));
    muteEffects = false;
    playRandomMusic = false;
    muteMusic = false;
    //music cross faid
    musicVolume = 0;
    musicQueue = MUSIC_NON;
    musicPlaying = MUSIC_NON;
    //
    Mix_Volume(-1, START_SOUND_EFFECTS_VOLUME);
    Mix_VolumeMusic(musicVolume);
    Mix_AllocateChannels(SOUNDS_EFFECTS_COUNT + MUSIC_COUNT);
}

void LoadSounds(void) {
    //===sound effects===
    ClearScreenSoildColor();
    DrawTextStandAlone(LOADING_TEXT_X, LOADING_TEXT_Y, LOADING_TEXT_SOUNDS);

    char * tmpDir = BufferStringMakeBaseDir(EFFECTS_TEST_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_TEST] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_BELLRING_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BELLRING] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_BOOM_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOOM] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }
    
    
    tmpDir = BufferStringMakeBaseDir(EFFECTS_BOUNCE1_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOUNCE1] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_BOUNCE2_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOUNCE2] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_BOUNCE3_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOUNCE3] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_CLICK_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_CLICK] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_CLICKCLOCK_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_CLICKCLOCK] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_DULESQEEK_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_DULESQEEK] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_HITBONG_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_HITBONG] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_PITTERPATTER_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_PITTERPATTER] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_QUICKPITTERPATTER_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_QUICKPITTERPATTER] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_REVERSEDHIT_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_REVERSEDHIT] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_SQEEKIN_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_SQEEKIN] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_SQEEKOUTFAST_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_SQEEKOUTFAST] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_SQEEKSLOW_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_SQEEKSLOW] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_WORBLE_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_WORBLE] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(EFFECTS_AIRHORN_FILE);
    if (NULL == (gameSoundEffects[SOUND_EFFECT_AIRHORN] = Mix_LoadWAV(tmpDir))) {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
        assert(false);
    }


    //===music===
    ClearScreenSoildColor();
    DrawTextStandAlone(LOADING_TEXT_X, LOADING_TEXT_Y, LOADING_TEXT_MUSIC);

    tmpDir = BufferStringMakeBaseDir(MUSIC_01_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_01] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_02_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_02] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_03_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_03] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_04_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_04] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_05_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_05] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_06_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_06] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_07_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_07] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_08_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_08] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_09_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_09] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_10_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_10] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }

    tmpDir = BufferStringMakeBaseDir(MUSIC_11_FILE);
    if (NULL == (gameMusic[SOUND_MUSIC_11] = Mix_LoadMUS(tmpDir))) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        assert(false);
    }
}

void PlayMusic(const uint8_t musicId) {
    if (
        !muteMusic
        && musicId != musicPlaying
        && musicId < MUSIC_COUNT
        ) 
    {
        musicQueue = musicId;
    }
}

void PlaySoundEffect(const uint8_t soundId) {
    //make sure its not alreay playing or muted
    if (
        false == Mix_Playing(soundId)
        && !muteEffects
        && soundId < SOUNDS_EFFECTS_COUNT
        )
    {
        Mix_PlayChannel(soundId, gameSoundEffects[soundId], 0);
    }
}

void StopSoundEffect(const uint8_t soundId) {
    //make sure its not alreay playing or muted
    if (soundId < SOUNDS_EFFECTS_COUNT)
    {
        Mix_HaltChannel(soundId);
    }
}

void PlayMusicRandom(void) {
    uint8_t id = Rng8() % MUSIC_COUNT;
    if (musicPlaying == id) {
        if (++id >= MUSIC_COUNT) {
            id = 0;
        }
    }
    PlayMusic(id);
}

void SetPlayRandomMusic(const bool setPlay) {
    playRandomMusic = setPlay;
    if (playRandomMusic) {
        PlayMusicRandom();
    }
}

void SetMuteSoundEffects(const bool mute) {
    muteEffects = mute;
    if (muteEffects) {
        for (uint8_t i = 0; i < SOUNDS_EFFECTS_COUNT; ++i) {
            Mix_HaltChannel(i);
        }
    }
}

void SetMuteMusic(const bool mute) {
    muteMusic = mute;
    if (muteMusic) {
        Mix_HaltMusic();
        musicVolume = 0;
        musicQueue = MUSIC_NON;
        musicPlaying = MUSIC_NON;
    }
}

void SoundStep(void) {
    //music crossfaid
    if (!muteMusic && musicQueue < MUSIC_COUNT) {
        //faid music out
        if (musicQueue != musicPlaying) {
            musicVolume -= MUSIC_FAID_SPEED;
            //done faiding, play new song
            if (musicVolume <= MUSIC_FAID_CUTOFF) {
                musicPlaying = musicQueue;
                musicVolume = 0;
                Mix_HaltMusic();
                Mix_PlayMusic(gameMusic[musicPlaying], 0);
            }
            Mix_VolumeMusic(musicVolume);
        }//faid music in 
        else if (musicVolume < MUSIC_MAX_VOLUME) {
            musicVolume += MUSIC_FAID_SPEED;
            if (musicVolume >= (MUSIC_MAX_VOLUME - MUSIC_FAID_CUTOFF)) {
                musicVolume = MUSIC_MAX_VOLUME;
            }
            Mix_VolumeMusic(musicVolume);
        }
        //random playlist
        if (playRandomMusic
            && musicQueue == musicPlaying
            && !Mix_PlayingMusic()) {
            musicVolume = 0;
            //move onto the next song on list
            if (++musicQueue >= MUSIC_COUNT) {
                musicQueue = 0;
            }
            PlayMusic(musicQueue);
        }
    }//end of crossfaid music
}

void CloseSound(void) {
    for (uint8_t i = 0; i < SOUNDS_EFFECTS_COUNT; ++i) {
        Mix_FreeChunk(gameSoundEffects[i]);
    }
    for (uint8_t i = 0; i < MUSIC_COUNT; ++i) {
        Mix_FreeMusic(gameMusic[i]);
    }
    Mix_CloseAudio();
    Mix_Quit();
}