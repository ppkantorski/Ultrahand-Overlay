/********************************************************************************
 * File: audio_player.cpp
 * Author: ppkantorski
 * Description:
 *   Memory-optimized version with reduced allocation overhead and chunked I/O.
 *   Key changes:
 *   - Eliminated temporary vector allocations (saves 50% memory during load)
 *   - Chunked file reading to reduce peak memory usage
 *   - Reduced alignment padding (saves ~3-4KB per sound)
 *   - Added lazy loading option via unloadAllSounds()
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2025 ppkantorski
 ********************************************************************************/

#include "audio_player.hpp"

namespace ult {
    bool AudioPlayer::m_initialized = false;
    std::atomic<bool> AudioPlayer::m_enabled{true};
    float AudioPlayer::m_masterVolume = 0.6f;
    bool AudioPlayer::m_lastDockedState = false;
    std::vector<AudioPlayer::CachedSound> AudioPlayer::m_cachedSounds;
    std::mutex AudioPlayer::m_audioMutex;
    
    bool AudioPlayer::initialize() {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        
        if (m_initialized) return true;
        
        if (R_FAILED(audoutInitialize()) || R_FAILED(audoutStartAudioOut())) {
            audoutExit();
            return false;
        }
        
        m_initialized = true;
        m_cachedSounds.resize(static_cast<uint32_t>(SoundType::Count));
        m_lastDockedState = ult::consoleIsDocked();
        reloadAllSounds();
        
        return true;
    }
    
    void AudioPlayer::exit() {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        
        // Free all cached sound buffers
        for (auto& cached : m_cachedSounds) {
            if (cached.buffer) {
                free(cached.buffer);
                cached.buffer = nullptr;
            }
            cached.bufferSize = 0;
            cached.dataSize = 0;
        }
        
        if (m_initialized) {
            audoutStopAudioOut();
            audoutExit();
            m_initialized = false;
        }
    }
    
    void AudioPlayer::reloadAllSounds() {
        for (uint32_t i = 0; i < static_cast<uint32_t>(SoundType::Count); ++i) {
            loadSoundFromWav(static_cast<SoundType>(i), m_soundPaths[i]);
        }
    }

    void AudioPlayer::unloadAllSounds(const std::initializer_list<SoundType>& excludeSounds) {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        if (!m_initialized) return;
    
        for (uint32_t i = 0; i < m_cachedSounds.size(); ++i) {
            SoundType current = static_cast<SoundType>(i);
    
            // Skip if this sound is in the exclude list
            if (std::find(excludeSounds.begin(), excludeSounds.end(), current) != excludeSounds.end()) {
                continue;
            }
    
            auto& cached = m_cachedSounds[i];
            if (cached.buffer) {
                free(cached.buffer);
                cached.buffer = nullptr;
            }
            cached.bufferSize = 0;
            cached.dataSize = 0;
        }
    }
    
    bool AudioPlayer::reloadIfDockedChanged() {
        if (!m_initialized) return false;
        
        const bool currentDocked = ult::consoleIsDocked();
        if (currentDocked == m_lastDockedState) return false;
        
        std::lock_guard<std::mutex> lock(m_audioMutex);
        m_lastDockedState = currentDocked;
        reloadAllSounds();
        
        return true;
    }
    
    bool AudioPlayer::loadSoundFromWav(SoundType type, const char* path) {
        const uint32_t idx = static_cast<uint32_t>(type);
        if (!m_initialized || idx >= static_cast<uint32_t>(SoundType::Count)) return false;
    
        // Free existing buffer
        free(m_cachedSounds[idx].buffer);
        m_cachedSounds[idx] = { nullptr, 0, 0 };
    
        FILE* f = fopen(path, "rb");
        if (!f) return false;
    
        // Parse WAV header
        char hdr[12];
        if (fread(hdr, 1, 12, f) != 12 || memcmp(hdr, "RIFF", 4) || memcmp(hdr + 8, "WAVE", 4)) {
            fclose(f);
            return false;
        }
    
        u16 fmt = 0, ch = 0, bits = 0;
        u32 rate = 0, dSize = 0;
        long dPos = 0;
    
        // Find fmt and data chunks
        while (fread(hdr, 1, 8, f) == 8) {
            const u32 sz = *(u32*)(hdr + 4);
            if (!memcmp(hdr, "fmt ", 4)) {
                fread(&fmt, 2, 1, f);
                fread(&ch, 2, 1, f);
                fread(&rate, 4, 1, f);
                fseek(f, 6, SEEK_CUR);
                fread(&bits, 2, 1, f);
                fseek(f, sz - 16, SEEK_CUR);
            } else if (!memcmp(hdr, "data", 4)) {
                dSize = sz;
                dPos = ftell(f);
                break;
            } else {
                fseek(f, sz, SEEK_CUR);
            }
        }
    
        // Validate format
        if (!dSize || fmt != 1 || ch == 0 || ch > 2 || (bits != 8 && bits != 16)) {
            fclose(f);
            return false;
        }
    
        // Calculate buffer sizes
        // Note: audout REQUIRES stereo (2 channels), so we must duplicate mono
        const bool mono = (ch == 1);
        const uint32_t inSamples = dSize / (bits / 8);
        const uint32_t outSamples = mono ? inSamples * 2 : inSamples;  // Duplicate mono to stereo
        const uint32_t outSize = outSamples * 2;  // 16-bit samples
        
        // Use smaller alignment to reduce waste (256 bytes instead of 4KB)
        const uint32_t align = 0x100;
        const uint32_t bufSize = (outSize + align - 1) & ~(align - 1);
    
        // Allocate output buffer
        void* buf = aligned_alloc(align, bufSize);
        if (!buf) {
            fclose(f);
            return false;
        }
    
        fseek(f, dPos, SEEK_SET);
        s16* out = (s16*)buf;
        
        // Calculate volume scaling
        float effectiveVolume = m_masterVolume;
        if (m_lastDockedState) {
            effectiveVolume *= 0.5f;
        }
        const float scale = std::clamp(effectiveVolume, 0.0f, 1.0f);
    
        // Process audio in chunks to minimize memory usage
        // This eliminates the need for temporary vectors
        constexpr uint32_t CHUNK_SIZE = 512;
        
        if (bits == 8) {
            // 8-bit audio: read and convert in chunks
            const int32_t scaleInt = static_cast<int32_t>(scale * 256.0f);
            u8 chunk[CHUNK_SIZE];
            uint32_t remaining = inSamples;
            uint32_t outIdx = 0;
            
            while (remaining > 0) {
                const uint32_t toRead = std::min(remaining, CHUNK_SIZE);
                if (fread(chunk, 1, toRead, f) != toRead) {
                    free(buf);
                    fclose(f);
                    return false;
                }
                
                for (uint32_t i = 0; i < toRead; ++i) {
                    const s16 sample = static_cast<s16>((chunk[i] - 128) * scaleInt);
                    if (mono) {
                        // Duplicate to both L and R channels for stereo output
                        out[outIdx++] = sample;  // Left
                        out[outIdx++] = sample;  // Right
                    } else {
                        out[outIdx++] = sample;
                    }
                }
                remaining -= toRead;
            }
        } else {
            // 16-bit audio: read and convert in chunks
            s16 chunk[CHUNK_SIZE];
            uint32_t remaining = inSamples;
            uint32_t outIdx = 0;
            
            while (remaining > 0) {
                const uint32_t toRead = std::min(remaining, CHUNK_SIZE);
                if (fread(chunk, sizeof(s16), toRead, f) != toRead) {
                    free(buf);
                    fclose(f);
                    return false;
                }
                
                for (uint32_t i = 0; i < toRead; ++i) {
                    const s16 sample = static_cast<s16>(chunk[i] * scale);
                    if (mono) {
                        // Duplicate to both L and R channels for stereo output
                        out[outIdx++] = sample;  // Left
                        out[outIdx++] = sample;  // Right
                    } else {
                        out[outIdx++] = sample;
                    }
                }
                remaining -= toRead;
            }
        }
    
        fclose(f);
    
        // Zero-fill any padding
        if (outSize < bufSize) {
            memset((u8*)buf + outSize, 0, bufSize - outSize);
        }
    
        m_cachedSounds[idx] = { buf, bufSize, outSize };
        return true;
    }
    
    void AudioPlayer::playSound(SoundType type) {
        // Lock-free check - SAFE with atomic
        if (!m_enabled.load(std::memory_order_relaxed)) return;
    
        const uint32_t idx = static_cast<uint32_t>(type);
        if (idx >= static_cast<uint32_t>(SoundType::Count)) return;
    
        std::lock_guard<std::mutex> lock(m_audioMutex);
        
        // Check again under lock
        if (!m_initialized) return;
        
        auto& cached = m_cachedSounds[idx];
        if (!cached.buffer) return;
    
        // Release any finished buffers
        AudioOutBuffer* releasedBuffers = nullptr;
        u32 releasedCount = 0;
        audoutGetReleasedAudioOutBuffer(&releasedBuffers, &releasedCount);
    
        // Static buffer is safe with mutex protection
        static AudioOutBuffer audioBuffer = {};
        audioBuffer = {};
        audioBuffer.buffer = cached.buffer;
        audioBuffer.buffer_size = cached.bufferSize;
        audioBuffer.data_size = cached.dataSize;
        audioBuffer.data_offset = 0;
        audioBuffer.next = nullptr;
    
        AudioOutBuffer* rel = nullptr;
        audoutPlayBuffer(&audioBuffer, &rel);
    }
    
    void AudioPlayer::setMasterVolume(float v) { 
        std::lock_guard<std::mutex> lock(m_audioMutex);
        m_masterVolume = std::clamp(v, 0.0f, 1.0f);
    }
    
    void AudioPlayer::setEnabled(bool e) { 
        m_enabled.store(e, std::memory_order_relaxed);
    }
    
    bool AudioPlayer::isEnabled() { 
        return m_enabled.load(std::memory_order_relaxed);
    }
    
   //bool AudioPlayer::isDocked() {
   //    Result rc = apmInitialize();
   //    if (R_FAILED(rc)) return false;
   //    
   //    ApmPerformanceMode perfMode = ApmPerformanceMode_Invalid;
   //    rc = apmGetPerformanceMode(&perfMode);
   //    apmExit();
   //    
   //    return R_SUCCEEDED(rc) && (perfMode == ApmPerformanceMode_Boost);
   //}
}