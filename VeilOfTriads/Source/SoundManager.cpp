#include "SoundManager.h"

using namespace ax;

bool SoundManager::_enabled = true;

int SoundManager::_musicId = -1;

void SoundManager::preload()
{
    AudioEngine::preload("res/Music/pop.ogg");
    AudioEngine::preload("res/Music/pixel-dungeon-sound.ogg");
}

void SoundManager::playPop()
{
    if (!_enabled) return;
    AudioEngine::play2d("res/Music/pop.ogg");
}

void SoundManager::playMusic()
{
    if (!_enabled) return;

    if (_musicId != -1)
      return;

    _musicId = AudioEngine::play2d("res/Music/pixel-dungeon-sound.ogg", true, 0.5f);
}

void SoundManager::setEnabled(bool enabled)
{
    _enabled = enabled;

    if (!enabled)
    {
        AudioEngine::stopAll();
        _musicId = -1;
    }
    else
    {
      playMusic();
    }
}

bool SoundManager::isEnabled()
{
    return _enabled;
}
