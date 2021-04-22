#ifndef SOUND_H
#define SOUND_H

#include "Graphics.h"

void LoadSounds(void) {

    //===sound effects===
    ClearScreenSoildColor();
    DrawTextStandAlone(LOADING_TEXT_X, LOADING_TEXT_Y, LOADING_TEXT_SOUNDS);

    if (NULL == (gameSoundEffects[SOUND_EFFECT_TEST]              = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_TEST_FILE             )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BELLRING]          = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_BELLRING_FILE         )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOOM]              = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_BOOM_FILE             )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOUNCE1]           = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_BOUNCE1_FILE          )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOUNCE2]           = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_BOUNCE2_FILE          )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_BOUNCE3]           = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_BOUNCE3_FILE          )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_CLICK]             = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_CLICK_FILE            )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_CLICKCLOCK]        = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_CLICKCLOCK_FILE       )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_DULESQEEK]         = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_DULESQEEK_FILE        )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_HITBONG]           = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_HITBONG_FILE          )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_PITTERPATTER]      = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_PITTERPATTER_FILE     )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_QUICKPITTERPATTER] = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_QUICKPITTERPATTER_FILE)))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_REVERSEDHIT]       = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_REVERSEDHIT_FILE      )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_SQEEKIN]           = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_SQEEKIN_FILE          )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_SQEEKOUTFAST]      = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_SQEEKOUTFAST_FILE     )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_SQEEKSLOW]         = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_SQEEKSLOW_FILE        )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_WORBLE]            = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_WORBLE_FILE           )))) goto SoundLoadError;
    if (NULL == (gameSoundEffects[SOUND_EFFECT_AIRHORN]           = Mix_LoadWAV(BufferStringMakeBaseDir(EFFECTS_AIRHORN_FILE          )))) goto SoundLoadError;

    //===music===
    ClearScreenSoildColor();
    DrawTextStandAlone(LOADING_TEXT_X, LOADING_TEXT_Y, LOADING_TEXT_MUSIC);

    if (NULL == (gameMusic[SOUND_MUSIC_01] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_01_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_02] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_02_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_03] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_03_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_04] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_04_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_05] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_05_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_06] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_06_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_07] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_07_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_08] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_08_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_09] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_09_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_10] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_10_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_11] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_11_FILE)))) goto SoundLoadError;

    if (NULL == (gameMusic[SOUND_MUSIC_12] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_12_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_13] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_13_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_14] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_14_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_15] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_15_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_16] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_16_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_17] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_17_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_18] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_18_FILE)))) goto SoundLoadError;
    if (NULL == (gameMusic[SOUND_MUSIC_19] = Mix_LoadMUS(BufferStringMakeBaseDir(MUSIC_19_FILE)))) goto SoundLoadError;

    //error handling
    return;
SoundLoadError:
    printf("Loading sound error: %s\n", Mix_GetError());
    assert(false);
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
    if (soundId < SOUNDS_EFFECTS_COUNT)
    {
        Mix_HaltChannel(soundId);
    }
}

void PlayMusicRandom(void) {
    PlayMusic((((uint8_t)musicPlaying + (Rng8() % (MUSIC_COUNT - 1))) % MUSIC_COUNT));
}

void SetPlayRandomMusic(const bool setPlay) {

    if ((playRandomMusic = setPlay) == true) {
        PlayMusicRandom();
    }
}

void SetMuteSoundEffects(const bool mute) {
    if ((muteEffects = mute) == true) {
        for (uint8_t i = 0; i < SOUNDS_EFFECTS_COUNT; ++i) {
            Mix_HaltChannel(i);
        }
    }
}

void SetMuteMusic(const bool mute) {
    if ((muteMusic = mute) == true) {
        Mix_HaltMusic();
        musicVolume  = 0;
        musicQueue   = MUSIC_NON;
        musicPlaying = MUSIC_NON;
    }
}

void SoundStep(void) {
    //music crossfaid
    if (!muteMusic && musicQueue < MUSIC_COUNT) {
        //faid music out
        if (musicQueue != musicPlaying) {
            musicVolume -= MUSIC_FAID_SPEED;
            //when done faiding, play new song
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

void InitSound(void) {
    //init SDL_mixer
    //https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_frame.html

    if (Mix_Init(SOUND_SDL_INIT_SETTINGS) != SOUND_SDL_INIT_SETTINGS) {
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
    if (Mix_OpenAudio(SOUND_BASE_SAMPLE_RATE, MIX_DEFAULT_FORMAT, SOUND_CHANNEL_COUNT, SOUND_SIMPLE_BUFFER_SIZE) == -1) {
        printf("%s", Mix_GetError());
        assert(false);
    }

    //set defaults
    ZeroOut((uint8_t*)gameMusic, sizeof(gameMusic));
    ZeroOut((uint8_t*)gameSoundEffects, sizeof(gameSoundEffects));
    muteEffects = false;
    playRandomMusic = false;
    muteMusic = false;

    //music crossfade
    musicVolume = 0;
    musicQueue = MUSIC_NON;
    musicPlaying = MUSIC_NON;

    //player/ball sound helpers
    playersRunningSound = false;
    playersWalkingSound = false;
    spriteTimerSound = 1;
    ballSounIndexOffset = 0;

    //sound mix settings
    Mix_Volume(-1, START_SOUND_EFFECTS_VOLUME);
    Mix_VolumeMusic(musicVolume);
    Mix_AllocateChannels(SOUNDS_EFFECTS_COUNT + MUSIC_COUNT);

    //load audio into memory
    LoadSounds();
}

#endif