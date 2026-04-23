#pragma once
#include "audio/AudioEngine.h"

class SoundManager {
public:
    static void preload();
    static void playPop();
    static void playMusic();

    static void setEnabled(bool enabled);
    static bool isEnabled();

private:
    static bool _enabled;
    static int _musicId;
};
