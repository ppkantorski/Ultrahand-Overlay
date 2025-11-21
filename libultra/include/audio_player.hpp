/********************************************************************************
 * File: audio_player.hpp
 * Author: ppkantorski
 * Description:
 *   This header defines the AudioPlayer class and related structures used for
 *   handling sound playback within the Ultrahand Overlay. It provides interfaces
 *   for loading, caching, and playing WAV audio through libnx’s audout service,
 *   along with basic sound type management and synchronization support.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *   Licensed under both GPLv2 and CC-BY-4.0
 *   Copyright (c) 2025 ppkantorski
 ********************************************************************************/

#pragma once
#include <switch.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <atomic>
#include <cstring>
#include <mutex>
#include "tsl_utils.hpp"

namespace ult {
    class AudioPlayer {
    public:
        enum class SoundType : uint8_t {  // <- uint8_t saves space
            Navigate,
            Enter,
            Exit,
            Wall,
            On,
            Off,
            Settings,
            Move,
            Count
        };
        
        struct CachedSound {
            void* buffer = nullptr;
            uint32_t bufferSize = 0;
            uint32_t dataSize = 0;
        };
        
        static bool initialize();
        static void exit();
        static void playSound(SoundType type);
        
        // Inline wrappers - same API, zero overhead
        static inline void playNavigateSound() { playSound(SoundType::Navigate); }
        static inline void playEnterSound() { playSound(SoundType::Enter); }
        static inline void playExitSound() { playSound(SoundType::Exit); }
        static inline void playWallSound() { playSound(SoundType::Wall); }
        static inline void playOnSound() { playSound(SoundType::On); }
        static inline void playOffSound() { playSound(SoundType::Off); }
        static inline void playSettingsSound() { playSound(SoundType::Settings); }
        static inline void playMoveSound() { playSound(SoundType::Move); }
        
        static void setMasterVolume(float volume);
        static void setEnabled(bool enabled);
        static bool isEnabled();
        //static bool isDocked();
        static bool reloadIfDockedChanged();
        static void reloadAllSounds();
        static void unloadAllSounds(const std::initializer_list<SoundType>& excludeSounds = {});
        static std::mutex m_audioMutex;
        
    private:
        static bool m_initialized;
        static std::atomic<bool> m_enabled;  // <- atomic for lock-free reads
        static float m_masterVolume;
        static std::vector<CachedSound> m_cachedSounds;
        
        static bool m_lastDockedState;
        
        inline static constexpr const char* m_soundPaths[static_cast<size_t>(SoundType::Count)] = {
            "sdmc:/config/ultrahand/sounds/tick.wav",
            "sdmc:/config/ultrahand/sounds/enter.wav",
            "sdmc:/config/ultrahand/sounds/exit.wav",
            "sdmc:/config/ultrahand/sounds/wall.wav",
            "sdmc:/config/ultrahand/sounds/on.wav",
            "sdmc:/config/ultrahand/sounds/off.wav",
            "sdmc:/config/ultrahand/sounds/settings.wav",
            "sdmc:/config/ultrahand/sounds/move.wav"
        };
        
        //static void playAudioBuffer(void* buffer, uint32_t bufferSize);
        static bool loadSoundFromWav(SoundType type, const char* path);
        
    };
}